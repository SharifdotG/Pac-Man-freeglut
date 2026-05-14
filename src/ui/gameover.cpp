#include "ui/gameover.h"

#include <GL/freeglut.h>

#include <cmath>
#include <cstdio>

#include "render/gl_init.h"
#include "render/primitives.h"
#include "render/text.h"

namespace ui {

namespace {

constexpr render::Color kOverlayColor = {0.00f, 0.00f, 0.00f, 0.65f};
constexpr render::Color kGameOverColor = {1.00f, 0.30f, 0.30f, 1.0f};
constexpr render::Color kWinColor = {0.30f, 1.00f, 0.40f, 1.0f};
constexpr render::Color kScoreColor = {1.00f, 0.95f, 0.30f, 1.0f};
constexpr render::Color kItemColor = {0.85f, 0.85f, 0.95f, 1.0f};
constexpr render::Color kItemSelectColor = {1.00f, 0.95f, 0.30f, 1.0f};
constexpr render::Color kFooterColor = {0.55f, 0.55f, 0.70f, 1.0f};

const char* gameover_label(int idx, bool win) {
    if (idx == static_cast<int>(GameOverItem::Retry)) {
        return win ? "PLAY AGAIN" : "RETRY";
    }
    if (idx == static_cast<int>(GameOverItem::Menu)) {
        return "MAIN MENU";
    }
    return "?";
}

void draw_screen(const char* title,
                 render::Color title_color,
                 int selected,
                 int final_score,
                 float play_time_seconds) {
    const float cx = static_cast<float>(render::kWindowWidth) * 0.5f;
    const float cy = static_cast<float>(render::kPlayAreaHeight) * 0.5f;

    // Translucent dark over the play area only — HUD stays clearly readable.
    render::set_color(kOverlayColor);
    render::draw_quad(0.0f,
                      0.0f,
                      static_cast<float>(render::kWindowWidth),
                      static_cast<float>(render::kPlayAreaHeight));

    render::set_color(title_color);
    render::draw_text_centered(cx, cy - 100.0f, title);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "FINAL SCORE: %d", final_score);
    render::set_color(kScoreColor);
    render::draw_text_centered(cx, cy - 60.0f, buf);

    {
        const int total_secs = static_cast<int>(play_time_seconds);
        const int mm = total_secs / 60;
        const int ss = total_secs % 60;
        std::snprintf(buf, sizeof(buf), "TIME: %02d:%02d", mm, ss);
        render::set_color({0.85f, 0.85f, 0.95f, 1.0f});
        render::draw_text_centered(cx, cy - 32.0f, buf);
    }

    // P12: accent line under the title.
    render::set_color({0.95f, 0.72f, 0.10f, 1.0f});
    render::draw_quad(cx - 60.0f, cy - 84.0f, 120.0f, 2.0f);

    const float pulse_t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
    const float pulse = 0.85f + 0.15f * std::sin(pulse_t * 6.0f);

    const int n = static_cast<int>(GameOverItem::Count);
    for (int i = 0; i < n; ++i) {
        const float y = cy + 30.0f + static_cast<float>(i) * 40.0f;  // P12: 36 → 40 px
        const bool is_sel = (i == selected);
        if (is_sel) {
            render::set_color({kItemSelectColor.r * pulse,
                               kItemSelectColor.g * pulse,
                               kItemSelectColor.b * pulse,
                               1.0f});
        } else {
            render::set_color(kItemColor);
        }
        char row[32];
        std::snprintf(row,
                      sizeof(row),
                      "%s  %s",
                      is_sel ? ">" : " ",
                      gameover_label(i, /*win=*/title_color.g > title_color.r));
        render::draw_text_centered(cx, y, row);
    }

    render::set_color(kFooterColor);
    render::draw_text_centered(cx, cy + 130.0f, "Arrow keys + Enter to choose");
}

}  // namespace

void render_game_over(int selected, int final_score, float play_time_seconds) {
    draw_screen("GAME OVER", kGameOverColor, selected, final_score, play_time_seconds);
}

void render_win_screen(int selected, int final_score, float play_time_seconds) {
    draw_screen("YOU WIN!", kWinColor, selected, final_score, play_time_seconds);
}

}  // namespace ui
