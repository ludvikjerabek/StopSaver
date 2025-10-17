#pragma once
#include <windows.h>
#include <stdexcept>

class SingleInstanceGuard {
public:
    explicit SingleInstanceGuard(const std::wstring& name)
        : _mutex(nullptr), _alreadyExists(false)
    {
        _mutex = CreateMutex(nullptr, FALSE, name.c_str());
        if (!_mutex) {
            throw std::runtime_error("Failed to create mutex");
        }

        _alreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);
    }

    ~SingleInstanceGuard() noexcept {
        if (_mutex) {
            CloseHandle(_mutex);
            _mutex = nullptr;
        }
    }

    // Non-copyable
    SingleInstanceGuard(const SingleInstanceGuard&) = delete;
    SingleInstanceGuard& operator=(const SingleInstanceGuard&) = delete;

    // Movable
    SingleInstanceGuard(SingleInstanceGuard&& other) noexcept
        : _mutex(other._mutex), _alreadyExists(other._alreadyExists)
    {
        other._mutex = nullptr;
    }

    bool alreadyExists() const noexcept { return _alreadyExists; }

private:
    HANDLE _mutex;
    bool _alreadyExists;
};
