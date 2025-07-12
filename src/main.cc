#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "window.h"
#include "graphics.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Window wnd = {};
    if (FAILED(window_init(&wnd, hInstance, nCmdShow))) {
        return 0;
    }

    Graphics graphics = {};
    if (FAILED(graphics_init(&graphics, wnd.handle))) {
        graphics_cleanup(&graphics);
        return 0;
    }

    MSG msg = {};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        render(&graphics);
    }

    graphics_cleanup(&graphics);
    return (int)msg.wParam;
}