#pragma once

#include "common.h"
#include "mymath.h"

struct Scene {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

bool scene_init(Scene* scene, i32 width, i32 height) {
    if (!scene) {
        return true;
    }

    scene->world = DirectX::XMMatrixIdentity();

    XMVECTOR eye = DirectX::XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
    XMVECTOR at = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    scene->view = DirectX::XMMatrixLookAtLH(eye, at, up);

    scene->projection = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, width / (f32)height, 0.01f, 100.0f);

    return false;
}