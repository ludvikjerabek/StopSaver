#include <windows.h>
#include <shellapi.h>
#include <wtsapi32.h>
#include "resource.h"

// Needed for session change notifications
#pragma comment(lib, "Wtsapi32.lib")

// Global variables
HINSTANCE hInst;
HWND hWnd;
NOTIFYICONDATA nid;
UINT_PTR timerID = 0;
bool isStarted = false;
HANDLE hMutex = NULL;

// Function prototypes
void SetupTrayIcon();
void SetupContextMenu();
void DestroyTrayIcon();
void OnTimer();
void OnStart();
void OnStop();
void SendMouseMove();
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void Cleanup();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {

    hMutex = CreateMutex(NULL, FALSE, L"StopSaverTrayAppMutex");
    if (hMutex == NULL) {
        MessageBox(NULL, L"Failed to create mutex.", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Check if another instance is running
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, L"Another instance of the application is already running.", L"Error", MB_ICONEXCLAMATION | MB_OK);
        CloseHandle(hMutex); // Close handle before returning
        return 0;
    }

    hInst = hInstance;

    // Register window class
    const wchar_t* szWindowClass = L"StopSaverTrayApp";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szWindowClass;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        CloseHandle(hMutex); // Close handle before returning
        return 0;
    }

    // Create hidden window
    hWnd = CreateWindowEx(0, szWindowClass, L"Stop Saver Tray App", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        CloseHandle(hMutex); // Close handle before returning
        return 0;
    }

    // Listen for session change notifications
    WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_THIS_SESSION);

    // Setup the tray icon
    SetupTrayIcon();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Do resource cleanup    
    Cleanup();

    return (int)msg.wParam;
}

void SetupTrayIcon() {
    OutputDebugString(L"SetupTrayIcon\n");
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    wcscpy_s(nid.szTip, L"Stop Saver (Inactive)");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void UpdateTrayIcon() {
    HICON hIcon = isStarted ? LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)) : LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    if (!hIcon) {
        OutputDebugString(L"Failed to load icon.\n");
        return;
    }

    nid.hIcon = hIcon;
    nid.uFlags = NIF_ICON | NIF_TIP;

    isStarted ? wcscpy_s(nid.szTip, L"Stop Saver (Active)") : wcscpy_s(nid.szTip, L"Stop Saver (Inactive)");

    if (!Shell_NotifyIcon(NIM_MODIFY, &nid)) {
        OutputDebugString(L"Failed to update tray icon.\n");
    }
}

void SetupContextMenu() {
    OutputDebugString(L"SetupContextMenu\n");
    // Check menu state and toggle accordingly
    UINT startFlag = (timerID != 0) ? MF_CHECKED : MF_UNCHECKED;
    UINT stopFlag = (timerID == 0) ? MF_CHECKED : MF_UNCHECKED;

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
    OutputDebugString(L"DestroyTrayIcon\n");
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
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
            PostQuitMessage(0);
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
            OutputDebugString(L"Session locked\n");
            // Handle session lock
            OnStop(); // Example: stop the timer on lock
            break;
        case WTS_SESSION_UNLOCK:
            OutputDebugString(L"Session unlocked\n");
            // Handle session unlock
            // OnStart(); // Example: start the timer on unlock
            break;
        case WTS_SESSION_LOGON:
            OutputDebugString(L"Session logon\n");
            // Handle session logon
            // OnStart(); // Example: start the timer on logon
            break;
        case WTS_SESSION_LOGOFF:
            OutputDebugString(L"Session logoff\n");
            // Handle session logoff
            // OnStop(); // Example: stop the timer on logoff
            break;
        }
        break;
    case WM_TIMER:
        switch (wParam) {
        case 1:
            OnTimer();
            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void OnTimer() {
    OutputDebugString(L"OnTimer\n");
    SendMouseMove();
}

void OnStart() {
    OutputDebugString(L"OnStart\n");
    EXECUTION_STATE execState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    if (execState == NULL) {
        OutputDebugString(L"Failed to set execution state\n");
    }
    timerID = SetTimer(hWnd, 1, 30000, NULL);
    isStarted = true;
    UpdateTrayIcon();
}

void OnStop() {
    OutputDebugString(L"OnStop\n");
    if (timerID != 0) {
        KillTimer(hWnd, timerID);
        timerID = 0;
    }
    EXECUTION_STATE execState = SetThreadExecutionState(ES_CONTINUOUS);
    if (execState == NULL) {
        OutputDebugString(L"Failed to set execution state\n");
    }
    isStarted = false;
    UpdateTrayIcon();
}

void SendMouseMove() {
    OutputDebugString(L"SendMouseMove\n");
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

void Cleanup() {
    OutputDebugString(L"Cleanup\n");
    if (timerID != 0) {
        KillTimer(hWnd, timerID);
    }
    WTSUnRegisterSessionNotification(hWnd);
    DestroyTrayIcon();
    if (hMutex) {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
}
