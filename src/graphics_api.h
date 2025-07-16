#pragma once

#include "common.h"
#include "mymath.h"
#include "d3d11_layer.h"

namespace graphics {

// globals
D3D11Layer d3d11_layer;

void clear(const Color& color) {}

void draw_text(const char* text, f32 x, f32 y, f32 size) {}

void draw_cube(const XMMATRIX& xform, const Color& color) {}

void draw_cubes(XMMATRIX* xforms, u32 count, const Color& color,
                const XMMATRIX& view, const XMMATRIX& proj) {}

void draw_gui_slider();

void present() {}

// internals
// Status _graphics_startup() {}
// void _graphics_shutdown() {}

}  // namespace graphics
