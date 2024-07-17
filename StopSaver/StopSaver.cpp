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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	CheckForExistingInstance();
	Initialize(hInstance);

	// Message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void CheckForExistingInstance() {
	hMutex = CreateMutex(NULL, FALSE, L"StopSaverTrayAppMutex");
	if (hMutex == NULL) {
		MessageBox(NULL, L"Failed to create mutex.", L"Error", MB_ICONEXCLAMATION | MB_OK);
		exit(0);
	}

	// Check if another instance is running
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"Another instance of the application is already running.", L"Error", MB_ICONEXCLAMATION | MB_OK);
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		exit(0);
	}
}

void Initialize(HINSTANCE hInstance) {
	hInst = hInstance;
	CreateMainWindow(hInstance);

	// Listen for session change notifications
	if (!WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_THIS_SESSION)) {
		MessageBox(NULL, L"Failed to register session notifications.", L"Error", MB_ICONEXCLAMATION | MB_OK);
		exit(0);
	}

	// Setup the tray icon
	SetupTrayIcon();
}


void CreateMainWindow(HINSTANCE hInstance) {
	// Register window class
	const wchar_t* szWindowClass = L"StopSaverTrayApp";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = szWindowClass;

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		CloseHandle(hMutex); // Close handle before returning
		exit(0);
	}

	// Create hidden window
	hWnd = CreateWindowEx(0, szWindowClass, L"Stop Saver Tray App", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) {
		MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		CloseHandle(hMutex); // Close handle before returning
		exit(0);
	}
}


void SetupTrayIcon() {
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_APP + 1;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
	wcscpy_s(nid.szTip, L"Stop Saver (Inactive)");
	if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
		OutputDebugString(L"Failed to add tray icon.\n");
	}
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
	// Check menu state and toggle accordingly
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
			OutputDebugString(L"Session locked\n");
			OnStop(); // Example: stop the timer on lock
			break;
		case WTS_SESSION_UNLOCK:
			OutputDebugString(L"Session unlocked\n");
			// OnStart(); // Example: start the timer on unlock (if needed)
			break;
		case WTS_SESSION_LOGON:
			OutputDebugString(L"Session logon\n");
			// OnStart(); // Example: start the timer on logon (if needed)
			break;
		case WTS_SESSION_LOGOFF:
			OutputDebugString(L"Session logoff\n");
			OnStop(); // Example: stop the timer on logoff
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
	SendMouseMove();
}


void OnStart() {
	EXECUTION_STATE execState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
	if (execState == NULL) {
		OutputDebugString(L"Failed to set execution state\n");
	}
	timerID = SetTimer(hWnd, 1, 30000, NULL);
	if (timerID == 0) {
		OutputDebugString(L"Failed to create timer\n");
	}
	else {
		isStarted = true;
		UpdateTrayIcon();
	}
}


void OnStop() {
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

void OnExit() {
	PostQuitMessage(0);
}

void SendMouseMove() {
	INPUT input = {};
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, &input, sizeof(INPUT));
}

void Cleanup() {
	if (timerID != 0) {
		KillTimer(hWnd, timerID);
		timerID = 0;
	}
	WTSUnRegisterSessionNotification(hWnd);
	DestroyTrayIcon();
	if (hMutex) {
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		hMutex = NULL;
	}
}
