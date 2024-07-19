#include <windows.h>
#include <shellapi.h>
#include <wtsapi32.h>
#include "resource.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"


// Needed for session change notifications
#pragma comment(lib, "Wtsapi32.lib")

// Global variables
HINSTANCE hInst;
HWND hWnd;
NOTIFYICONDATA nid;
UINT_PTR timerID = 0;
bool isStarted = false;
HANDLE hMutex = NULL;

std::shared_ptr<spdlog::logger> logger;

// Function prototypes
void CheckForExistingInstance();
void Initialize(HINSTANCE hInstance);
void CreateMainWindow(HINSTANCE hInstance);
void SetupTrayIcon();
void UpdateTrayIcon();
void SetupContextMenu();
void DestroyTrayIcon();
void OnTimer();
void OnStart();
void OnStop();
void OnExit();
void Cleanup();
void SendMouseMove();
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

std::wstring ExpandPath(const std::wstring& path) {
    wchar_t expandedPath[MAX_PATH];

    if (ExpandEnvironmentStrings(path.c_str(), expandedPath, MAX_PATH) == 0) {
        return L"";
    }

    return std::wstring(expandedPath);
}

spdlog::level::level_enum GetLogLevelFromEnv() {
    wchar_t* logLevelStr = nullptr;
    const spdlog::level::level_enum default_log_level = spdlog::level::err;
    size_t len;
    errno_t error = _wdupenv_s(&logLevelStr, &len, L"STOPSAVER_LOG_LEVEL");

    if (error != 0 || logLevelStr == nullptr) {
        return default_log_level;
    }

    std::wstring logLevelWStr(logLevelStr);
    std::string logLevel(logLevelWStr.begin(), logLevelWStr.end());

    free(logLevelStr);

    if (logLevel == "trace") return spdlog::level::trace;
    if (logLevel == "debug") return spdlog::level::debug;
    if (logLevel == "info") return spdlog::level::info;
    if (logLevel == "warn") return spdlog::level::warn;
    if (logLevel == "error") return spdlog::level::err;
    if (logLevel == "critical") return spdlog::level::critical;
    if (logLevel == "off") return spdlog::level::off;

    return default_log_level;
}


int WINAPI wWinMain (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd){
    std::wstring logPath = ExpandPath(L"%USERPROFILE%\\stopsaver.log");

    if (logPath.empty()) {
        MessageBox(NULL, L"Failed to expand log path.", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    try {
        logger = spdlog::rotating_logger_mt("stopsaver", logPath, 1 * 1024 * 1024, 1);
        logger->set_level(GetLogLevelFromEnv());
        logger->set_pattern("%Y-%m-%dT%H:%M:%S.%f%z %l %P: %v");
        logger->info(L"Application started");
    }
    catch (const spdlog::spdlog_ex& ex) {
        MessageBox(NULL, L"Log initialization failed.", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    CheckForExistingInstance();
    Initialize(hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    logger->info(L"Application exited");
    return (int)msg.wParam;
}

void CheckForExistingInstance() {
    logger->trace(L"CheckForExistingInstance()");
    hMutex = CreateMutex(NULL, FALSE, L"StopSaverTrayAppMutex");
    if (hMutex == NULL) {
        logger->error(L"Failed to create mutex");
        exit(0);
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        logger->error(L"Another instance of the application is already running");
        CloseHandle(hMutex);
        exit(0);
    }
}

void Initialize(HINSTANCE hInstance) {
    logger->trace(L"Initialize()");
    hInst = hInstance;
    CreateMainWindow(hInstance);

    if (!WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_THIS_SESSION)) {
        logger->error(L"Failed to register session notifications");
        exit(0);
    }

    SetupTrayIcon();
}

void CreateMainWindow(HINSTANCE hInstance) {
    logger->trace(L"CreateMainWindow()");
    const wchar_t* szWindowClass = L"StopSaverTrayApp";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szWindowClass;

    if (!RegisterClass(&wc)) {
        logger->error(L"Window Registration Failed!");
        CloseHandle(hMutex);
        exit(0);
    }

    hWnd = CreateWindowEx(0, szWindowClass, L"Stop Saver Tray App", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        logger->error(L"Window Creation Failed!");
        CloseHandle(hMutex);
        exit(0);
    }
}

void SetupTrayIcon() {
    logger->trace(L"SetupTrayIcon()");
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    wcscpy_s(nid.szTip, L"Stop Saver (Inactive)");
    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        logger->error(L"Failed to add tray icon");
    }
}

void UpdateTrayIcon() {
    logger->trace(L"UpdateTrayIcon()");
    HICON hIcon = isStarted ? LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)) : LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    if (!hIcon) {
        logger->error(L"Failed to load icon");
        return;
    }

    nid.hIcon = hIcon;
    nid.uFlags = NIF_ICON | NIF_TIP;

    if (isStarted) {
        wcscpy_s(nid.szTip, L"Stop Saver (Active)");
    }
    else {
        wcscpy_s(nid.szTip, L"Stop Saver (Inactive)");
    }

    if (!Shell_NotifyIcon(NIM_MODIFY, &nid)) {
        logger->error(L"Failed to update tray icon");
    }
}

void SetupContextMenu() {
    logger->trace(L"SetupContextMenu()");
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | (isStarted ? MF_CHECKED : MF_UNCHECKED), 1, L"Start");
    AppendMenu(hMenu, MF_STRING | (!isStarted ? MF_CHECKED : MF_UNCHECKED), 2, L"Stop");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 4, L"Exit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
}

void DestroyTrayIcon() {
    logger->trace(L"DestroyTrayIcon()");
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
        Cleanup();
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // Start
            if (!isStarted) {
                OnStart();
            }
            break;
        case 2: // Stop
            if (isStarted) {
                OnStop();
            }
            break;
        case 4: // Exit
            OnExit();
            break;
        }
        break;
    case WM_APP + 1:
        if (lParam == WM_LBUTTONDOWN) {
            SetupContextMenu();
        }
        break;
    case WM_WTSSESSION_CHANGE:
        switch (wParam) {
        case WTS_SESSION_LOCK:
            logger->debug(L"Session locked");
            OnStop();
            break;
        case WTS_SESSION_UNLOCK:
            logger->debug(L"Session unlocked");
            // OnStart();
            break;
        case WTS_SESSION_LOGON:
            logger->debug(L"Session logon");
            // OnStart();
            break;
        case WTS_SESSION_LOGOFF:
            logger->debug(L"Session logoff");
            OnStop();
            break;
        }
        break;
    case WM_TIMER:
        if (wParam == 1) {
            OnTimer();
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void OnTimer() {
    logger->trace(L"OnTimer()");
    SendMouseMove();
}

void OnStart() {
    logger->trace(L"OnStart()");
    EXECUTION_STATE execState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    if (execState == NULL) {
        logger->error(L"Failed to set execution state");
    }
    timerID = SetTimer(hWnd, 1, 30000, NULL);
    if (timerID == 0) {
        logger->error(L"Failed to create timer");
    }
    else {
        isStarted = true;
        UpdateTrayIcon();
    }
}

void OnStop() {
    logger->trace(L"OnStop()");
    if (timerID != 0) {
        KillTimer(hWnd, timerID);
        timerID = 0;
    }
    EXECUTION_STATE execState = SetThreadExecutionState(ES_CONTINUOUS);
    if (execState == NULL) {
        logger->error(L"Failed to set execution state");
    }
    isStarted = false;
    UpdateTrayIcon();
}

void OnExit() {
    logger->trace(L"OnExit()");
    PostQuitMessage(0);
}

void SendMouseMove() {
    logger->trace(L"SendMouseMove()");
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

void Cleanup() {
    logger->trace(L"Cleanup()");
    if (timerID != 0) {
        KillTimer(hWnd, timerID);
        timerID = 0;
    }
    WTSUnRegisterSessionNotification(hWnd);
    DestroyTrayIcon();
    if (hMutex) {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
}
