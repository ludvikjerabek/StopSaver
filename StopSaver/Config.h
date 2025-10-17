#pragma once
#include <string>
#include <spdlog/fwd.h>           
#include "winreg/WinReg.hpp"      

class Config {
public:
    Config();

    std::wstring getLogFile(const std::wstring& def = L"%USERPROFILE%\\stopsaver.log") const noexcept;
    void setLogFile(const std::wstring& logFileName);

    spdlog::level::level_enum getLogLevel(const std::wstring& def = L"error") const noexcept;
    void setLogLevel(const std::wstring& logLevel);

    std::uint32_t getMaxSizeLogSize(std::uint32_t def = 10u * 1024u * 1024u) const noexcept;
    void setMaxSizeLogSize(std::uint32_t logFileSize);

    std::uint32_t getMouseIntervalMs(std::uint32_t def = 30000) const noexcept;
    void setMouseIntervalMs(std::uint32_t interval);
    
    bool getAutoStartOnLaunch(bool def = false) const noexcept;
    void setAutoStartOnLaunch(bool bAutoStart);

    bool getRestoreOnUnlock(bool def = false) const noexcept;
    void setRestoreOnUnlock(bool bRestore);

    bool valueExists(const wchar_t* name) const noexcept;
    void deleteValue(const wchar_t* name);

private:
    std::wstring getOrDefaultString(const std::wstring& name,
        const std::wstring& def) const noexcept;

    std::wstring getOrDefaultExpand(const std::wstring& name,
        const std::wstring& def) const noexcept;

private:
    winreg::RegKey _config_key;
};