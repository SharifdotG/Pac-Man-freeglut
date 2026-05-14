#include "ui/menu.h"

#include <GL/freeglut.h>

#include <cmath>
#include <cstdio>

#include "render/gl_init.h"
#include "render/primitives.h"
#include "render/text.h"

namespace ui {

namespace {

constexpr render::Color kTitleColor = {1.00f, 0.92f, 0.16f, 1.0f};  // Pac yellow
constexpr render::Color kItemColor = {0.85f, 0.85f, 0.95f, 1.0f};
constexpr render::Color kItemSelectColor = {1.00f, 0.95f, 0.30f, 1.0f};
constexpr render::Color kFooterColor = {0.50f, 0.50f, 0.65f, 1.0f};
constexpr render::Color kAccentColor = {0.95f, 0.72f, 0.10f, 1.0f};
constexpr render::Color kDotColor = {1.00f, 0.85f, 0.62f, 1.0f};

const char* item_label(int idx) {
    switch (idx) {
        case static_cast<int>(MainMenuItem::Start):
            return "START";
        case static_cast<int>(MainMenuItem::Help):
            return "HELP";
        case static_cast<int>(MainMenuItem::Settings):
            return "AUDIO SETTINGS";
        case static_cast<int>(MainMenuItem::Exit):
            return "EXIT";
    }
    return "?";
}

// P12: gentle heartbeat for selected menu items. Returns a multiplier in
// [0.85, 1.00] driven by wall-clock time — sub-frame precision means the
// pulse looks smooth even at 60 Hz.
float pulse_multiplier() {
    const float t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
    return 0.85f + 0.15f * std::sin(t * 6.0f);
}

// P12: draw two small Pac-style dots flanking the centred title text.
// Pure procedural — the spacing is computed from the text width so the
// dots line up correctly regardless of the title.
void draw_title_dots(float cx, float y, const char* title, render::Color color) {
    const int half_w = render::text_width_px(title) / 2;
    const float dx = static_cast<float>(half_w) + 24.0f;
    render::set_color(color);
    render::draw_filled_circle(cx - dx, y - 6.0f, 4.5f, 16);
    render::draw_filled_circle(cx + dx, y - 6.0f, 4.5f, 16);
}

}  // namespace

void render_main_menu(int selected, int hi_score) {
    const float cx = static_cast<float>(render::kWindowWidth) * 0.5f;

    // === Title with decorative dots ===
    render::set_color(kTitleColor);
    render::draw_text_centered(cx, 180.0f, "PAC-MAN");
    draw_title_dots(cx, 180.0f, "PAC-MAN", kDotColor);

    render::set_color({0.55f, 0.55f, 0.75f, 1.0f});
    render::draw_text_centered(cx, 210.0f, "Made by Sharif, Priom & Ovi");

    // Accent line under the subtitle.
    render::set_color(kAccentColor);
    render::draw_quad(cx - 60.0f, 226.0f, 120.0f, 2.0f);

    // === Menu items (heartbeat pulse on the selected row) ===
    const int n = static_cast<int>(MainMenuItem::Count);
    for (int i = 0; i < n; ++i) {
        const float y = 320.0f + static_cast<float>(i) * 48.0f;  // P12: 40 → 48 px spacing
        const bool is_sel = (i == selected);
        if (is_sel) {
            const float p = pulse_multiplier();
            render::set_color(
                {kItemSelectColor.r * p, kItemSelectColor.g * p, kItemSelectColor.b * p, 1.0f});
        } else {
            render::set_color(kItemColor);
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%s  %s", is_sel ? ">" : " ", item_label(i));
        render::draw_text_centered(cx, y, buf);
    }

    // === High-score readout (skip when 0 to keep the menu clean on first run) ===
    if (hi_score > 0) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "HIGH SCORE: %d", hi_score);
        render::set_color(kFooterColor);
        render::draw_text_centered(cx, 560.0f, buf);
    }

    // === Footer / hint ===
    render::set_color(kFooterColor);
    render::draw_text_centered(cx, 700.0f, "Use Arrow keys / WASD + Enter to navigate");
    render::draw_text_centered(cx, 724.0f, "CSE 426 Computer Graphics Lab — Spring 2025");
}

}  // namespace ui
