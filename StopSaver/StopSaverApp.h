#pragma once
#include <windows.h>
#include "resource.h"
#include "TrayIcon.h"
#include "Timer.h"
#include "SessionNotify.h"
#include "HMenuPopup.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "Config.h"

class StopSaverApp 
{
public:
    explicit StopSaverApp(std::shared_ptr<Config> config, std::shared_ptr<spdlog::logger> log) : _config(std::move(config)), _logger(std::move(log)) {}

    ~StopSaverApp() = default;

    bool init(HINSTANCE hInst);
    int  run();

private:
    // RAII for RegisterClass/UnregisterClass
    class WindowClassRegistrar {
        HINSTANCE _hInst{};
        const wchar_t* _cls{};
        bool _ok{};
    public:
        WindowClassRegistrar(HINSTANCE hInst, const wchar_t* cls, WNDPROC proc)
            : _hInst(hInst), _cls(cls) {
            WNDCLASSW wc{};
            wc.lpfnWndProc = proc;
            wc.hInstance = hInst;
            wc.lpszClassName = cls;
            _ok = !!RegisterClassW(&wc);
        }
        ~WindowClassRegistrar() {
            if (_ok) UnregisterClassW(_cls, _hInst);
        }
        bool ok() const { return _ok; }
    };

    bool createMainWindow();
    void updateTrayIcon();
    void setupContextMenu();
    void onTimer();
    void onStart();
    void onStop();
    void onAutoStart();
    void onRestoreOnUnlock();
    void onShowUserAsActive();
    void onExit();
    void sendMouseMove();

    // Static thunk and instance proc
    static LRESULT CALLBACK s_wndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE _hInst{ nullptr };
    HWND      _hWnd{ nullptr };
    bool      _isStarted{ false };
    bool      _stateOnLock{ false };
    bool      _autoStartOnLaunch{ false };
    bool      _restoreOnUnlock{ false };
    bool      _showUserAsActive{ false };
    UINT      _timer_interval{ 30000 };

    TrayIcon       _tray;
    Timer          _timer;
    SessionNotify  _session;

    std::unique_ptr<WindowClassRegistrar> _wcReg;
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<Config> _config;
};
