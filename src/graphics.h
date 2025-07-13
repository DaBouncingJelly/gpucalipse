#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

#include "mymath.h"

struct Graphics {
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swap_chain;
    ID3D11RenderTargetView* rtv;
    ID3D11DepthStencilView* dsv;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* il;
    ID3D11Buffer* vb;
    ID3D11Buffer* ib;
    ID3D11Buffer* cb;

    u32 width;
    u32 height;
};

struct Vertex {
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct ConstantBuffer {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX proj;
};

const char* g_vertexShaderSource = R"(
cbuffer ConstantBuffer : register(b0) {
    matrix world;
    matrix view;
    matrix proj;
}

struct VS_INPUT {
    float4 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, world);
    output.Pos = mul(output.Pos, view);
    output.Pos = mul(output.Pos, proj);
    output.Color = input.Color;
    return output;
}
)";

const char* g_pixelShaderSource = R"(
struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

float4 PS(PS_INPUT input) : SV_Target {
    return input.Color;
}
)";

HRESULT CompileShaderFromSource(const char* szShader, LPCSTR szEntryPoint,
                                LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

HRESULT graphics_init(Graphics* graphics, HWND window_handle) {
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(window_handle, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    graphics->width = width;
    graphics->height = height;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes;
         driverTypeIndex++) {
        D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags,
                               featureLevels, numFeatureLevels,
                               D3D11_SDK_VERSION, &graphics->device, nullptr,
                               &graphics->context);

        if (hr == E_INVALIDARG) {
            hr = D3D11CreateDevice(
                nullptr, driverType, nullptr, createDeviceFlags,
                &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION,
                &graphics->device, nullptr, &graphics->context);
        }

        if (SUCCEEDED(hr)) {
            break;
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = graphics->device->QueryInterface(
            __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr)) {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr)) {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1),
                                        reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window_handle;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = dxgiFactory->CreateSwapChain(graphics->device, &sd,
                                      &graphics->swap_chain);
    dxgiFactory->Release();
    if (FAILED(hr)) {
        return hr;
    }

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = graphics->swap_chain->GetBuffer(
        0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr)) {
        return hr;
    }

    hr = graphics->device->CreateRenderTargetView(pBackBuffer, nullptr,
                                                  &graphics->rtv);
    pBackBuffer->Release();
    if (FAILED(hr)) {
        return hr;
    }

    ID3D11Texture2D* pDepthStencil = nullptr;
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = graphics->device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
    if (FAILED(hr)) {
        return hr;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = graphics->device->CreateDepthStencilView(pDepthStencil, &descDSV,
                                                  &graphics->dsv);
    pDepthStencil->Release();
    if (FAILED(hr)) {
        return hr;
    }

    graphics->context->OMSetRenderTargets(1, &graphics->rtv, graphics->dsv);

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    graphics->context->RSSetViewports(1, &vp);

    ID3DBlob* pVSBlob = nullptr;
    hr =
        CompileShaderFromSource(g_vertexShaderSource, "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr)) return hr;

    hr = graphics->device->CreateVertexShader(pVSBlob->GetBufferPointer(),
                                              pVSBlob->GetBufferSize(), nullptr,
                                              &graphics->vs);
    if (FAILED(hr)) {
        pVSBlob->Release();
        return hr;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    UINT numElements = ARRAYSIZE(layout);

    hr = graphics->device->CreateInputLayout(
        layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &graphics->il);
    pVSBlob->Release();
    if (FAILED(hr)) {
        return hr;
    }

    graphics->context->IASetInputLayout(graphics->il);

    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromSource(g_pixelShaderSource, "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr)) {
        return hr;
    }

    hr = graphics->device->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                             pPSBlob->GetBufferSize(), nullptr,
                                             &graphics->ps);
    pPSBlob->Release();
    if (FAILED(hr)) {
        return hr;
    }

    Vertex vertices[] = {
        // Front face
        {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
        {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},

        // Back face
        {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
        {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f)},
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    hr = graphics->device->CreateBuffer(&bd, &InitData, &graphics->vb);
    if (FAILED(hr)) {
        return hr;
    }

    WORD indices[] = {// Front face  (z = -1)
                      0, 1, 2, 0, 2, 3,

                      // Back face   (z = +1)
                      4, 5, 6, 4, 6, 7,

                      // Left face   (x = -1)
                      4, 7, 1, 4, 1, 0,

                      // Right face  (x = +1)
                      3, 2, 6, 3, 6, 5,

                      // Top face    (y = +1)
                      1, 7, 6, 1, 6, 2,

                      // Bottom face (y = -1)
                      4, 0, 3, 4, 3, 5};

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = graphics->device->CreateBuffer(&bd, &InitData, &graphics->ib);
    if (FAILED(hr)) {
        return hr;
    }

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = graphics->device->CreateBuffer(&bd, nullptr, &graphics->cb);
    if (FAILED(hr)) {
        return hr;
    }

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    graphics->context->IASetVertexBuffers(0, 1, &graphics->vb, &stride,
                                          &offset);
    graphics->context->IASetIndexBuffer(graphics->ib, DXGI_FORMAT_R16_UINT, 0);
    graphics->context->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return S_OK;
}

void graphics_cleanup(Graphics* graphics) {
    if (!graphics) {
        return;
    }
    if (graphics->context) graphics->context->ClearState();
    if (graphics->cb) graphics->cb->Release();
    if (graphics->ib) graphics->ib->Release();
    if (graphics->vb) graphics->vb->Release();
    if (graphics->il) graphics->il->Release();
    if (graphics->vs) graphics->vs->Release();
    if (graphics->ps) graphics->ps->Release();
    if (graphics->dsv) graphics->dsv->Release();
    if (graphics->rtv) graphics->rtv->Release();
    if (graphics->swap_chain) graphics->swap_chain->Release();
    if (graphics->context) graphics->context->Release();
    if (graphics->device) graphics->device->Release();
}

HRESULT CompileShaderFromSource(const char* szShader, LPCSTR szEntryPoint,
                                LPCSTR szShaderModel, ID3DBlob** ppBlobOut) {
    HRESULT hr = S_OK;
    ID3DBlob* pErrorBlob = nullptr;

    hr = D3DCompile(szShader, strlen(szShader), nullptr, nullptr, nullptr,
                    szEntryPoint, szShaderModel, D3DCOMPILE_ENABLE_STRICTNESS,
                    0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr)) {
        if (pErrorBlob) {
            OutputDebugStringA(
                reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}