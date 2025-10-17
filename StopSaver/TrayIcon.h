#pragma once
#include <windows.h>

class TrayIcon {
    NOTIFYICONDATAW nid{};
public:
    TrayIcon() noexcept = default;

    bool init(HINSTANCE hInst, HWND hWnd, UINT iconResInactive, UINT callbackMsg, const wchar_t* tip) {
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = callbackMsg;
        nid.hIcon = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(iconResInactive), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        if (!nid.hIcon) return false;
        wcsncpy_s(nid.szTip, tip, _TRUNCATE);
        return Shell_NotifyIconW(NIM_ADD, &nid) != FALSE;
    }

    void setActiveState(HINSTANCE hInst, bool isStarted, UINT iconResActive, UINT iconResInactive,
        const wchar_t* tipActive, const wchar_t* tipInactive) {
        HICON hIcon = (HICON)LoadImageW(
            hInst, MAKEINTRESOURCEW(isStarted ? iconResActive : iconResInactive),
            IMAGE_ICON, 0, 0, LR_DEFAULTSIZE
        );
        if (!hIcon) return;
        nid.hIcon = hIcon;
        nid.uFlags = NIF_ICON | NIF_TIP;
        wcsncpy_s(nid.szTip, isStarted ? tipActive : tipInactive, _TRUNCATE);
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

    ~TrayIcon() {
        if (nid.hWnd) Shell_NotifyIconW(NIM_DELETE, &nid);
    }

    TrayIcon(const TrayIcon&) = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;
    TrayIcon(TrayIcon&&) = delete;
    TrayIcon& operator=(TrayIcon&&) = delete;
};
