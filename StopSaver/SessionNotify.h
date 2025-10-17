#pragma once
#include <windows.h>
#include <wtsapi32.h>

#pragma comment(lib, "Wtsapi32.lib")

class SessionNotify {
    HWND _hWnd{};
public:
    SessionNotify() noexcept = default;

    bool registerFor(HWND hWnd) {
        _hWnd = hWnd;
        return WTSRegisterSessionNotification(_hWnd, NOTIFY_FOR_THIS_SESSION) != FALSE;
    }
    ~SessionNotify() {
        if (_hWnd) WTSUnRegisterSessionNotification(_hWnd);
    }

    SessionNotify(const SessionNotify&) = delete;
    SessionNotify& operator=(const SessionNotify&) = delete;
    SessionNotify(SessionNotify&&) = delete;
    SessionNotify& operator=(SessionNotify&&) = delete;
};
