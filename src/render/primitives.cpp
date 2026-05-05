#include "render/primitives.h"

#include <GL/freeglut.h>

#include <cmath>

namespace render {

void set_color(Color c) {
    glColor4f(c.r, c.g, c.b, c.a);
}

void draw_quad(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void draw_line(float x0, float y0, float x1, float y1, float thickness) {
    // Lines as thin quads. glLineWidth() works but its handling of
    // non-1.0 widths varies by driver; quads are predictable.
    const float dx = x1 - x0;
    const float dy = y1 - y0;
    const float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.0f) {
        return;
    }
    const float nx = -dy / len * (thickness * 0.5f);
    const float ny = dx / len * (thickness * 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(x0 + nx, y0 + ny);
    glVertex2f(x1 + nx, y1 + ny);
    glVertex2f(x1 - nx, y1 - ny);
    glVertex2f(x0 - nx, y0 - ny);
    glEnd();
}

void draw_filled_circle(float cx, float cy, float radius, int segments) {
    if (segments < 3)
        segments = 3;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    const float two_pi = 6.28318530717958647692f;
    for (int i = 0; i <= segments; ++i) {
        const float t = two_pi * static_cast<float>(i) / static_cast<float>(segments);
        glVertex2f(cx + std::cos(t) * radius, cy + std::sin(t) * radius);
    }
    glEnd();
}

}  // namespace render
