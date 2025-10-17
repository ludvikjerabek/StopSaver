#pragma once
#include <windows.h>

class Timer {
    HWND _hWnd{};
    UINT_PTR _id{};
public:
    Timer() noexcept = default;

    bool start(HWND hWnd, UINT_PTR id, UINT ms, TIMERPROC proc = nullptr) {
        _hWnd = hWnd;
        _id = SetTimer(_hWnd, id, ms, proc);
        return _id != 0;
    }
    void stop() {
        if (_id) { KillTimer(_hWnd, _id); _id = 0; }
    }
    ~Timer() { stop(); }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;
};
