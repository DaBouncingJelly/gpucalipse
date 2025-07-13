#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "window.h"
#include "graphics.h"
#include "scene.h"

void update_and_render(Graphics* graphics, Scene* scene);

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

    Scene scene = {};
    if (FAILED(scene_init(&scene, graphics.width, graphics.height))) {
        return 0;
    }

    MSG msg = {};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        update_and_render(&graphics, &scene);
    }

    graphics_cleanup(&graphics);
    return (int)msg.wParam;
}

void update_and_render(Graphics* graphics, Scene* scene) {
    static float t = 0.0f;
    t += 0.00016f;
    scene->world = DirectX::XMMatrixRotationY(t);

    float color[4] = {0.0f, 0.125f, 0.3f, 1.0f};
    graphics->context->ClearRenderTargetView(graphics->rtv, color);
    graphics->context->ClearDepthStencilView(graphics->dsv, D3D11_CLEAR_DEPTH,
                                             1.0f, 0);

    ConstantBuffer cb;
    cb.world = DirectX::XMMatrixTranspose(scene->world);
    cb.view = DirectX::XMMatrixTranspose(scene->view);
    cb.proj = DirectX::XMMatrixTranspose(scene->projection);
    graphics->context->UpdateSubresource(graphics->cb, 0, nullptr, &cb, 0, 0);

    graphics->context->VSSetShader(graphics->vs, nullptr, 0);
    graphics->context->VSSetConstantBuffers(0, 1, &graphics->cb);
    graphics->context->PSSetShader(graphics->ps, nullptr, 0);
    graphics->context->DrawIndexed(36, 0, 0);
    graphics->swap_chain->Present(0, 0);
}