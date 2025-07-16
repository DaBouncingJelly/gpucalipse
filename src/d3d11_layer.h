#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "common.h"

namespace graphics {

struct D3D11Layer {
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swap_chain;
    ID3D11RenderTargetView* rtv;
    ID3D11DepthStencilView* dsv;

    u32 width;
    u32 height;
};

HRESULT d3d11layer_startup(D3D11Layer* layer, u32 width, u32 height) {
    IDXGIFactory1* factory = nullptr;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1),
                                    reinterpret_cast<void**>(&factory));
    if (FAILED(hr)) {
        return hr;
    }

    IDXGIAdapter1* adapter = nullptr;
    hr = factory->EnumAdapters1(0, &adapter);
    if (FAILED(hr)) {
        return hr;
    }

    u32 flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL feature_level;
    hr = FAILED(D3D11CreateDevice(
        adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, nullptr, 0,
        D3D11_SDK_VERSION, &layer->device, &feature_level, &layer->context));
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

void d3d11layer_shutdown() {}

HRESULT compile_shader_from_source(const char* shader, LPCSTR entry_point,
                                   LPCSTR shader_model, ID3DBlob** out) {
    HRESULT hr = S_OK;
    ID3DBlob* error = nullptr;

    hr = D3DCompile(shader, strlen(shader), nullptr, nullptr, nullptr,
                    entry_point, shader_model, D3DCOMPILE_ENABLE_STRICTNESS, 0,
                    out, &error);

    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA(
                reinterpret_cast<const char*>(error->GetBufferPointer()));
            error->Release();
        }
        return hr;
    }

    if (error) {
        error->Release();
    }

    return S_OK;
}

}  // namespace graphics