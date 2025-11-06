#pragma once
#include <windows.h>

class TrayIcon {
    static constexpr wchar_t kTipActive[] = L"Stop Saver (Active)";
    static constexpr wchar_t kTipInactive[] = L"Stop Saver (Inactive)";
    NOTIFYICONDATAW nid{};
    UINT  _cbMsg = 0;
    HICON _hIconActive = nullptr;
    HICON _hIconInactive = nullptr;
    bool  _isActive = false;

public:
    TrayIcon() noexcept = default;

	// Initializes the tray icon with inactive state
    bool init(HINSTANCE hInst, HWND hWnd, UINT activeId, UINT inactiveID, UINT callbackMsg) {
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = callbackMsg;
        _cbMsg = callbackMsg;

		// Small icons by default, but there may be a need to perform DPI scaling in the future    
        const int cx = GetSystemMetrics(SM_CXSMICON);
        const int cy = GetSystemMetrics(SM_CYSMICON);

        _hIconActive = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(activeId), IMAGE_ICON, cx, cy, 0);
        _hIconInactive = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(inactiveID), IMAGE_ICON, cx, cy, 0);

        if (!_hIconInactive || !_hIconActive) return false;

        _isActive = false;
        nid.hIcon = _hIconInactive;
        wcsncpy_s(nid.szTip, kTipInactive, _TRUNCATE);

        if (!Shell_NotifyIconW(NIM_ADD, &nid)) return false;
        
        return true;
    }

	// Used to switch between active/inactive icon and tooltip
    void setActive(bool active) {
        if (!nid.hWnd) return;
        _isActive = active;
        nid.hIcon = _isActive ? _hIconActive : _hIconInactive;
        nid.uFlags = NIF_ICON | NIF_TIP;
        wcsncpy_s(nid.szTip, _isActive ? kTipActive : kTipInactive, _TRUNCATE);
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

	// Used to re-add the icon after Explorer restarts
    bool reAdd() {
        if (!nid.hWnd) return false;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = _cbMsg;
        nid.hIcon = _isActive ? _hIconActive : _hIconInactive;
        wcsncpy_s(nid.szTip, _isActive ? kTipActive : kTipInactive, _TRUNCATE);
        if (!Shell_NotifyIconW(NIM_ADD, &nid)) return false;
        return true;
    }

	// Destructor removes the icon from the tray
    ~TrayIcon() {
        if (nid.hWnd) Shell_NotifyIconW(NIM_DELETE, &nid);
        if (_hIconActive)   DestroyIcon(_hIconActive);
        if (_hIconInactive) DestroyIcon(_hIconInactive);
    }

    TrayIcon(const TrayIcon&) = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;
    TrayIcon(TrayIcon&&) = delete;
    TrayIcon& operator=(TrayIcon&&) = delete;
};
