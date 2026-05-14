#include "ui/pause.h"

#include <GL/freeglut.h>

#include <cmath>
#include <cstdio>

#include "render/gl_init.h"
#include "render/primitives.h"
#include "render/text.h"

namespace ui {

namespace {

constexpr render::Color kOverlayColor = {0.00f, 0.00f, 0.00f, 0.65f};
constexpr render::Color kHeaderColor = {1.00f, 0.92f, 0.16f, 1.0f};
constexpr render::Color kItemColor = {0.85f, 0.85f, 0.95f, 1.0f};
constexpr render::Color kItemSelectColor = {1.00f, 0.95f, 0.30f, 1.0f};
constexpr render::Color kFooterColor = {0.55f, 0.55f, 0.70f, 1.0f};

const char* item_label(int idx) {
    switch (idx) {
        case static_cast<int>(PauseItem::Resume):
            return "RESUME";
        case static_cast<int>(PauseItem::Menu):
            return "MAIN MENU";
    }
    return "?";
}

}  // namespace

void render_pause_overlay(int selected) {
    const float cx = static_cast<float>(render::kWindowWidth) * 0.5f;
    const float cy = static_cast<float>(render::kPlayAreaHeight) * 0.5f;

    // Translucent dark over the play area. Skips the HUD strip so score /
    // lives stay legible during the pause.
    render::set_color(kOverlayColor);
    render::draw_quad(0.0f,
                      0.0f,
                      static_cast<float>(render::kWindowWidth),
                      static_cast<float>(render::kPlayAreaHeight));

    render::set_color(kHeaderColor);
    render::draw_text_centered(cx, cy - 70.0f, "PAUSED");

    // P12: accent line under the header.
    render::set_color({0.95f, 0.72f, 0.10f, 1.0f});
    render::draw_quad(cx - 50.0f, cy - 56.0f, 100.0f, 2.0f);

    const float pulse_t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
    const float pulse = 0.85f + 0.15f * std::sin(pulse_t * 6.0f);

    const int n = static_cast<int>(PauseItem::Count);
    for (int i = 0; i < n; ++i) {
        const float y = cy + static_cast<float>(i) * 40.0f;  // P12: 36 → 40 px spacing
        const bool is_sel = (i == selected);
        if (is_sel) {
            render::set_color({kItemSelectColor.r * pulse,
                               kItemSelectColor.g * pulse,
                               kItemSelectColor.b * pulse,
                               1.0f});
        } else {
            render::set_color(kItemColor);
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%s  %s", is_sel ? ">" : " ", item_label(i));
        render::draw_text_centered(cx, y, buf);
    }

    render::set_color(kFooterColor);
    render::draw_text_centered(cx, cy + 130.0f, "P or Esc to resume");
}

}  // namespace ui
