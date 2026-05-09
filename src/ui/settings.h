#pragma once

#include <GL/freeglut.h>

#include <cmath>
#include <cstdio>

#include "audio/audio.h"
#include "render/gl_init.h"
#include "render/primitives.h"
#include "render/text.h"

namespace ui {

// Settings (audio) screen. Three slider rows; the Game state machine
// drives navigation (up/down selects, left/right adjusts in 5% steps).
enum class SettingsRow : int {
    Master = 0,
    Sfx = 1,
    Bgm = 2,
    Count = 3,
};

constexpr float kAudioStep = 0.05f;

inline void render_settings_screen(int selected_row) {
    const float cx = static_cast<float>(render::kWindowWidth) * 0.5f;

    constexpr render::Color kHeader = {1.00f, 0.92f, 0.16f, 1.0f};
    constexpr render::Color kLabel = {0.85f, 0.85f, 0.95f, 1.0f};
    constexpr render::Color kSelLab = {1.00f, 0.95f, 0.30f, 1.0f};
    constexpr render::Color kBarOff = {0.20f, 0.20f, 0.32f, 1.0f};
    constexpr render::Color kBarOn = {1.00f, 0.92f, 0.16f, 1.0f};
    constexpr render::Color kFooter = {0.55f, 0.55f, 0.70f, 1.0f};

    render::set_color(kHeader);
    render::draw_text_centered(cx, 120.0f, "AUDIO SETTINGS");
    // P12: accent line under the title.
    render::set_color({0.95f, 0.72f, 0.10f, 1.0f});
    render::draw_quad(cx - 80.0f, 136.0f, 160.0f, 2.0f);

    const float pulse_t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
    const float pulse = 0.85f + 0.15f * std::sin(pulse_t * 6.0f);

    struct Row {
        const char* label;
        float value;
    };
    const Row rows[] = {
        {"MASTER", audio::master_volume()},
        {"SFX", audio::sfx_volume()},
        {"BGM", audio::bgm_volume()},
    };

    constexpr float kBarX = 220.0f;
    constexpr float kBarW = 280.0f;
    constexpr float kBarH = 14.0f;
    constexpr float kRowDy = 70.0f;
    const float kStartY = 240.0f;

    for (int i = 0; i < 3; ++i) {
        const float y = kStartY + static_cast<float>(i) * kRowDy;
        const bool sel = (i == selected_row);

        // Label (selected row pulses gently)
        if (sel) {
            render::set_color({kSelLab.r * pulse, kSelLab.g * pulse, kSelLab.b * pulse, 1.0f});
        } else {
            render::set_color(kLabel);
        }
        char marker[3] = "  ";
        if (sel) {
            marker[0] = '>';
        }
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%s %s", marker, rows[i].label);
        render::draw_text(140.0f, y, buf);

        // Slider track
        render::set_color(kBarOff);
        render::draw_quad(kBarX, y - 12.0f, kBarW, kBarH);

        // Filled portion
        const float fill_w = kBarW * rows[i].value;
        render::set_color(kBarOn);
        render::draw_quad(kBarX, y - 12.0f, fill_w, kBarH);

        // Numeric percentage
        std::snprintf(buf, sizeof(buf), "%3d%%", static_cast<int>(rows[i].value * 100.0f + 0.5f));
        render::set_color(sel ? kSelLab : kLabel);
        render::draw_text(kBarX + kBarW + 16.0f, y, buf);
    }

    render::set_color(kFooter);
    render::draw_text_centered(cx, 530.0f, "Up/Down to select  Left/Right to adjust");
    render::draw_text_centered(cx, 720.0f, "Esc to return");
}

}  // namespace ui
