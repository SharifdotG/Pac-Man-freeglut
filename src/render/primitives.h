#pragma once

namespace render {

// Solid-color RGBA used by every primitive call below. Components are 0..1.
struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

// Pixel-coordinate primitives in the active ortho projection (top-left
// origin). All immediate-mode (glBegin/glEnd) — fine for a few hundred
// shapes per frame at 60 Hz.

void set_color(Color c);

void draw_quad(float x, float y, float w, float h);

void draw_line(float x0, float y0, float x1, float y1, float thickness = 2.0f);

void draw_filled_circle(float cx, float cy, float radius, int segments = 18);

}  // namespace render
