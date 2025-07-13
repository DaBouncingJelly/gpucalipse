#pragma once

#include "common.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Window {
    HWND handle;
};

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam,
                          LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HRESULT window_init(Window* wnd, HINSTANCE hInstance, int nCmdShow) {
    if (!wnd) {
        return E_FAIL;
    }
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wnd_proc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = "gpucalipseclass";

    if (!RegisterClassExA(&wcex)) {
        return E_FAIL;
    }

    wnd->handle = CreateWindowA(
        "gpucalipseclass", "gpucalipse", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
        CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!wnd->handle) {
        return E_FAIL;
    };

    ShowWindow(wnd->handle, nCmdShow);
    UpdateWindow(wnd->handle);

    return S_OK;
}

void update_window(Window& window) {}
