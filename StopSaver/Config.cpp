#include "Config.h"
#include <windows.h>
#include "spdlog/spdlog.h"

// Local to this TU: not visible from the header
static std::wstring ExpandEnvStrings(const std::wstring& input) {
    DWORD requiredSize = ::ExpandEnvironmentStringsW(input.c_str(), nullptr, 0);
    if (requiredSize == 0) {
        throw std::runtime_error("ExpandEnvironmentStringsW failed (size query).");
    }

    std::wstring result(requiredSize, L'\0');
    DWORD charsWritten = ::ExpandEnvironmentStringsW(input.c_str(), result.data(), requiredSize);
    if (charsWritten == 0 || charsWritten > requiredSize) {
        throw std::runtime_error("ExpandEnvironmentStringsW failed (expansion).");
    }
    result.resize(charsWritten - 1);
    return result;
}

Config::Config()
    : _config_key(HKEY_CURRENT_USER, L"SOFTWARE\\StopSaver")
{
}

std::wstring Config::getLogFile(const std::wstring& def) const noexcept {
    return getOrDefaultExpand(L"LogFile", def);
}

void Config::setLogFile(const std::wstring& logFileName) {
    _config_key.SetExpandStringValue(L"LogFile", logFileName);
}

spdlog::level::level_enum Config::getLogLevel(const std::wstring& def) const noexcept {
    const auto v = getOrDefaultString(L"LogLevel", def);
    if (v == L"trace")    return spdlog::level::trace;
    if (v == L"debug")    return spdlog::level::debug;
    if (v == L"info")     return spdlog::level::info;
    if (v == L"warn")     return spdlog::level::warn;
    if (v == L"error")    return spdlog::level::err;
    if (v == L"critical") return spdlog::level::critical;
    if (v == L"off")      return spdlog::level::off;
    return spdlog::level::err;
}

void Config::setLogLevel(const std::wstring& logLevel) {
    _config_key.SetStringValue(L"LogLevel", logLevel);
}

std::uint32_t Config::getMaxSizeLogSize(std::uint32_t def) const noexcept {
    try { return _config_key.GetDwordValue(L"MaxSizeLogSize"); }
    catch (...) { return def; }
}

void Config::setMaxSizeLogSize(std::uint32_t logFileSize) {
    _config_key.SetDwordValue(L"MaxSizeLogSize", logFileSize);
}

std::uint32_t Config::getMouseIntervalMs(std::uint32_t def) const noexcept {
    try { return _config_key.GetDwordValue(L"MouseIntervalMs"); }
    catch (...) { return def; }
}

void Config::setMouseIntervalMs(std::uint32_t interval){
    _config_key.SetDwordValue(L"MouseIntervalMs", interval);
}

bool Config::getAutoStartOnLaunch(bool def) const noexcept {
    try { return _config_key.GetDwordValue(L"AutoStartOnLaunch") != 0; }
    catch (...) { return def; }
}

void Config::setAutoStartOnLaunch(bool bAutoStart) {
    _config_key.SetDwordValue(L"AutoStartOnLaunch", bAutoStart ? 1u : 0u);
}

bool Config::getRestoreOnUnlock(bool def) const noexcept {
    try { return _config_key.GetDwordValue(L"RestoreOnUnlock") != 0; }
    catch (...) { return def; }
}

void Config::setRestoreOnUnlock(bool bRestore) {
    _config_key.SetDwordValue(L"RestoreOnUnlock", bRestore ? 1u : 0u);
}

bool Config::getShowUserAsActive(bool def) const noexcept {
    try { return _config_key.GetDwordValue(L"ShowUserAsActive") != 0; }
    catch (...) { return def; }
}

void Config::setShowUserAsActive(bool bShowActive) {
    _config_key.SetDwordValue(L"ShowUserAsActive", bShowActive ? 1u : 0u);
}

bool Config::valueExists(const wchar_t* name) const noexcept {
    try { (void)_config_key.QueryValueType(name); return true; }
    catch (...) { return false; }
}

void Config::deleteValue(const wchar_t* name) {
    _config_key.DeleteValue(name);
}

std::wstring Config::getOrDefaultString(const std::wstring& name,
    const std::wstring& def) const noexcept {
    try { return _config_key.GetStringValue(name); }
    catch (...) { return def; }
}

std::wstring Config::getOrDefaultExpand(const std::wstring& name,
    const std::wstring& def) const noexcept {
    try { return ExpandEnvStrings(_config_key.GetExpandStringValue(name)); }
    catch (...) { return ExpandEnvStrings(def); }
}
