#include "StopSaverApp.h"

bool StopSaverApp::init(HINSTANCE hInst) {
    _logger->trace(L"StopSaverApp::init()");
    _hInst = hInst;
    
    constexpr uint32_t MIN_INTERVAL_MS = 1000;
    constexpr uint32_t MAX_INTERVAL_MS = 60000;

    _timer_interval = _config->getMouseIntervalMs();

    if (_timer_interval < MIN_INTERVAL_MS) {
        _logger->warn(L"Mouse interval < 1000 ms — clamping to 1000 ms (1s)");
        _config->setMouseIntervalMs(MIN_INTERVAL_MS);
        _timer_interval = MIN_INTERVAL_MS;
    }
    else if (_timer_interval > MAX_INTERVAL_MS) {
        _logger->warn(L"Mouse interval > 60000 ms — clamping to 60000 ms (60s)");
        _config->setMouseIntervalMs(MAX_INTERVAL_MS);
        _timer_interval = MAX_INTERVAL_MS;
    }

    // Register window class with static thunk
    static const wchar_t* kClass = L"StopSaverTrayApp";
    _wcReg = std::make_unique<WindowClassRegistrar>(_hInst, kClass, &StopSaverApp::s_wndProc);
    if (!_wcReg->ok()) {
        _logger->error(L"Window Registration Failed");
        return false;
    }

    if (!createMainWindow()) return false;

    if (!_session.registerFor(_hWnd)) {
        _logger->error(L"Failed to register session notifications");
        return false;
    }

    if (!_tray.init(_hInst, _hWnd, IDI_ICON2, WM_APP + 1, L"Stop Saver (Inactive)")) {
        _logger->error(L"Failed to add tray icon");
        return false;
    }

    if (_config->getAutoStartOnLaunch()) {
        onStart();
    }

    return true;
}

int StopSaverApp::run() {
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    // Timer and tray and session unregister via destructors
    return static_cast<int>(msg.wParam);
}

bool StopSaverApp::createMainWindow() {
    _logger->trace(L"StopSaverApp::createMainWindow()");
    static const wchar_t* kClass = L"StopSaverTrayApp";

    // Pass `this` through lpParam. Bind in WM_NCCREATE.
    _hWnd = CreateWindowExW(
        0, kClass, L"Stop Saver Tray App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, _hInst, this
    );

    if (!_hWnd) {
        _logger->error(L"Window Creation Failed");
        return false;
    }
    return true;
}

void StopSaverApp::updateTrayIcon() {
    _logger->trace(L"StopSaverApp::updateTrayIcon()");
    _tray.setActiveState(
        _hInst,
        _isStarted,
        IDI_ICON1, IDI_ICON2,
        L"Stop Saver (Active)",
        L"Stop Saver (Inactive)"
    );
}

void StopSaverApp::setupContextMenu() {
    _logger->trace(L"StopSaverApp::setupContextMenu()");
    MenuPopup menu(_hInst, IDR_TRAYMENU);
    if (!menu.valid()) {
        _logger->error(L"Failed to load tray menu");
        return;
    }
    
    CheckMenuItem(menu, IDM_START, MF_BYCOMMAND | (_isStarted ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_STOP, MF_BYCOMMAND | (!_isStarted ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_AUTO_START, MF_BYCOMMAND | (_config->getAutoStartOnLaunch() ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, IDM_RESTORE_ON_UNLOCK, MF_BYCOMMAND | (_config->getRestoreOnUnlock() ? MF_CHECKED : MF_UNCHECKED));

    POINT pt; GetCursorPos(&pt);
    SetForegroundWindow(_hWnd);
    TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, _hWnd, nullptr);
    PostMessageW(_hWnd, WM_NULL, 0, 0); // ensures dismissal
}

void StopSaverApp::onTimer() {
    _logger->trace(L"StopSaverApp::onTimer()");
    sendMouseMove();
}

void StopSaverApp::onStart() {
    _logger->trace(L"StopSaverApp::onStart()");
    EXECUTION_STATE es = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    if (es == NULL) _logger->error(L"Failed to set execution state");

    if (!_timer.start(_hWnd, 1, _timer_interval)) {
        _logger->error(L"Failed to create timer");
        return;
    }

    _isStarted = true;
    updateTrayIcon();
}

void StopSaverApp::onStop() {
    _logger->trace(L"StopSaverApp::onStop()");
    _timer.stop();

    EXECUTION_STATE es = SetThreadExecutionState(ES_CONTINUOUS);
    if (es == NULL) _logger->error(L"Failed to set execution state");

    _isStarted = false;
    updateTrayIcon();
}

void StopSaverApp::onExit() {
    _logger->trace(L"StopSaverApp::onExit()");
    PostQuitMessage(0);
}

void StopSaverApp::onAutoStart() {
    _logger->trace(L"StopSaverApp::onAutoStart()");
    _config->setAutoStartOnLaunch(!_config->getAutoStartOnLaunch());
}

void StopSaverApp::onRestoreOnUnlock() {
    _logger->trace(L"StopSaverApp::onRestoreOnUnlock()");
    _config->setRestoreOnUnlock(!_config->getRestoreOnUnlock());
}

void StopSaverApp::sendMouseMove() {
    _logger->trace(L"StopSaverApp::sendMouseMove()");
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

LRESULT CALLBACK StopSaverApp::s_wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    StopSaverApp* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<StopSaverApp*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        // store HWND as soon as we have it
        if (self) self->_hWnd = hWnd;
    }
    else {
        self = reinterpret_cast<StopSaverApp*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (self) {
        return self->wndProc(msg, wParam, lParam);
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT StopSaverApp::wndProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_START: if (!_isStarted) onStart(); break;
        case IDM_STOP:  if (_isStarted)  onStop();  break;
        case IDM_AUTO_START: 
            onAutoStart();
            break;
        case IDM_RESTORE_ON_UNLOCK:
            onRestoreOnUnlock();
            break;
        case IDM_EXIT:  onExit(); break;
        }
        break;

    case WM_APP + 1:
        if (lParam == WM_LBUTTONDOWN) {
            setupContextMenu();
        }
        break;

    case WM_WTSSESSION_CHANGE:
        switch (wParam) {
        case WTS_SESSION_LOCK:   
            _logger->debug(L"Session locked");
            // Keep state when system is locked
            _stateOnLock = _isStarted;
            onStop();
            break;
        case WTS_SESSION_UNLOCK: 
            _logger->debug(L"Session unlocked");
            if (_config->getRestoreOnUnlock() && _stateOnLock)
            {
                _logger->debug(L"Restoring state, to running");
                onStart();
            }
            break;
        case WTS_SESSION_LOGON:  
            _logger->debug(L"Session logon");
            break;
        case WTS_SESSION_LOGOFF: 
            _logger->debug(L"Session logoff");   
            onStop(); 
            break;
        }
        break;

    case WM_TIMER:
        if (wParam == 1) onTimer();
        break;

    default:
        return DefWindowProcW(_hWnd, message, wParam, lParam);
    }
    return 0;
}
