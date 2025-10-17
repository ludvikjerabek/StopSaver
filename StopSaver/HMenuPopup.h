#pragma once
#include <windows.h>

class MenuPopup {
    HMENU _menu{ nullptr };
    HMENU _popup{ nullptr };
public:
    MenuPopup(HINSTANCE hInst, UINT menuResId) {
        _menu = LoadMenuW(hInst, MAKEINTRESOURCEW(menuResId));
        if (_menu) _popup = GetSubMenu(_menu, 0);
    }
    ~MenuPopup() {
        if (_menu) DestroyMenu(_menu);
    }
    bool valid() const { return _popup != nullptr; }
    operator HMENU() const { return _popup; } 
    MenuPopup(const MenuPopup&) = delete;
    MenuPopup& operator=(const MenuPopup&) = delete;
    MenuPopup(MenuPopup&&) = delete;
    MenuPopup& operator=(MenuPopup&&) = delete;
};