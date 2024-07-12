#include <windows.h>
#include <shellapi.h>
#include "resource.h"

// Global variables
HINSTANCE hInst;
HWND hWnd;
NOTIFYICONDATA nid;
UINT_PTR timerID;
bool isStarted = false;

// Function prototypes
void SetupTrayIcon();
void SetupContextMenu();
void DestroyTrayIcon();
void OnTimer();
void OnStart();
void OnStop();
void SendMouseMove();
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;

    // Register window class
    const char* szWindowClass = "StopSaverTrayApp";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szWindowClass;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create hidden window
    hWnd = CreateWindowEx(0, szWindowClass, "Stop Saver Tray App", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Setup the tray icon
    SetupTrayIcon();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void SetupTrayIcon() {
    OutputDebugString("SetupTrayIcon\n");
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1; 
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    strcpy_s(nid.szTip, "Stop Saver (Inactive)");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void UpdateTrayIcon() {
    HICON hIcon = isStarted ? LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)) : LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    if (!hIcon) {
        OutputDebugString("Failed to load icon.\n");
        return;
    }

    nid.hIcon = hIcon;
    nid.uFlags = NIF_ICON | NIF_TIP;
    
    isStarted ? strcpy_s(nid.szTip, "Stop Saver (Active)") : strcpy_s(nid.szTip, "Stop Saver (Inactive)");

    if (!Shell_NotifyIcon(NIM_MODIFY, &nid)) {
        OutputDebugString("Failed to update tray icon.\n");
    }
}

void SetupContextMenu() {
    OutputDebugString("SetupContextMenu\n");
    // Check menu state and toggle accordingly
    UINT startFlag = (timerID != 0) ? MF_CHECKED : MF_UNCHECKED;
    UINT stopFlag = (timerID == 0) ? MF_CHECKED : MF_UNCHECKED;

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | (isStarted ? MF_CHECKED : MF_UNCHECKED), 1, "Start");
    AppendMenu(hMenu, MF_STRING | (!isStarted ? MF_CHECKED : MF_UNCHECKED), 2, "Stop");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 4, "Exit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
}

void DestroyTrayIcon() {
    OutputDebugString("DestroyTrayIcon\n");
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
            DestroyTrayIcon();
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_APP + 1:
        if (lParam == WM_LBUTTONDOWN) {
            SetupContextMenu();
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
    OutputDebugString("OnTimer\n");
    SendMouseMove();
}

void OnStart() {
    OutputDebugString("OnStart\n");
    EXECUTION_STATE execState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    if (execState == NULL)
    {
        OutputDebugString("Failed to set execution state\n");
    }
    timerID = SetTimer(hWnd, 1, 30000, NULL);
    isStarted = true;
    UpdateTrayIcon();
}

void OnStop() {
    OutputDebugString("OnStop\n");
    KillTimer(hWnd, timerID);
    EXECUTION_STATE execState = SetThreadExecutionState(ES_CONTINUOUS);
    if (execState == NULL)
    {
        OutputDebugString("Failed to set execution state\n");
    }
    isStarted = false;
    UpdateTrayIcon();
}

void SendMouseMove() {
    OutputDebugString("OnMove\n");
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}
