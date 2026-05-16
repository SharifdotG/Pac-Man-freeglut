#include "ui/hud.h"

#include <GL/freeglut.h>

#include <cmath>
#include <cstdio>

#include "render/gl_init.h"
#include "render/primitives.h"
#include "render/text.h"
#include "world/tile.h"

namespace ui {

namespace {

// HUD lives in the bottom 80 px of the window: world y in [744, 824].
constexpr float kHudTopY = static_cast<float>(world::kPlayAreaHeight);  // 744
constexpr float kHudHeightPx = static_cast<float>(render::kHudHeight);  // 80

constexpr render::Color kSeparatorColor = {0.20f, 0.20f, 0.40f, 1.0f};
constexpr render::Color kLabelColor = {0.95f, 0.95f, 0.62f, 1.0f};  // soft yellow
constexpr render::Color kValueColor = {1.00f, 1.00f, 1.00f, 1.0f};  // white
constexpr render::Color kPacColor = {1.00f, 0.92f, 0.16f, 1.0f};

// Mini-Pacman icon for the lives display. Mouth opens to the right.
void draw_life_icon(float cx, float cy) {
    constexpr float kRadius = 8.0f;
    constexpr float kMouthHalf = 0.45f;  // radians; matches Pac's chomping pose
    constexpr int kSegments = 18;
    constexpr float kPi = 3.14159265358979323846f;

    render::set_color(kPacColor);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    const float arc_start = 0.0f + kMouthHalf;
    const float arc_end = kPi * 2.0f - kMouthHalf;
    for (int i = 0; i <= kSegments; ++i) {
        const float t = arc_start + (arc_end - arc_start) * static_cast<float>(i) /
                                        static_cast<float>(kSegments);
        glVertex2f(cx + std::cos(t) * kRadius, cy + std::sin(t) * kRadius);
    }
    glEnd();
}

}  // namespace

void render_hud(const gameplay::Score& s,
                const std::optional<gameplay::ActivePerk>& active_perk,
                float play_time_seconds) {
    // Subtle separator line between play area and HUD.
    render::set_color(kSeparatorColor);
    render::draw_line(8.0f,
                      kHudTopY + 2.0f,
                      static_cast<float>(world::kPlayAreaWidth) - 8.0f,
                      kHudTopY + 2.0f,
                      1.5f);

    char buf[32];

    // === LEFT BLOCK: score ===
    const float left_x = 24.0f;
    render::set_color(kLabelColor);
    render::draw_text(left_x, kHudTopY + 26.0f, "1UP");
    render::set_color(kValueColor);
    std::snprintf(buf, sizeof(buf), "%d", s.points());
    render::draw_text(left_x, kHudTopY + 50.0f, buf);

    // === CENTER BLOCK: hi-score ===
    const float center_x = static_cast<float>(world::kPlayAreaWidth) * 0.5f;
    render::set_color(kLabelColor);
    render::draw_text_centered(center_x, kHudTopY + 26.0f, "HIGH SCORE");
    render::set_color(kValueColor);
    std::snprintf(buf, sizeof(buf), "%d", s.hi_score());
    render::draw_text_centered(center_x, kHudTopY + 50.0f, buf);

    // === RIGHT BLOCK: level ===
    std::snprintf(buf, sizeof(buf), "LEVEL %d", s.level());
    const int level_w = render::text_width_px(buf);
    const float level_x =
        static_cast<float>(world::kPlayAreaWidth) - 24.0f - static_cast<float>(level_w);
    render::set_color(kLabelColor);
    render::draw_text(level_x, kHudTopY + 26.0f, buf);

    // === LIVES (below the score block, left-aligned) ===
    for (int i = 0; i < s.lives(); ++i) {
        const float cx = left_x + 8.0f + static_cast<float>(i) * 22.0f;
        const float cy = kHudTopY + 70.0f;
        draw_life_icon(cx, cy);
    }

    // === TIME readout (centred, below HIGH SCORE) ===
    {
        const int total_secs = static_cast<int>(play_time_seconds);
        const int mm = total_secs / 60;
        const int ss = total_secs % 60;
        std::snprintf(buf, sizeof(buf), "TIME  %02d:%02d", mm, ss);
        render::set_color(kLabelColor);
        render::draw_text_centered(center_x, kHudTopY + 70.0f, buf);
    }

    // === ACTIVE PERK (right block, below LEVEL) ===
    if (active_perk) {
        const float total = gameplay::perk_duration(active_perk->kind);
        const float frac = total > 0.0f ? (active_perk->remaining / total) : 0.0f;
        const float icon_cx = static_cast<float>(world::kPlayAreaWidth) - 32.0f;
        const float icon_cy = kHudTopY + 65.0f;
        gameplay::perk_draw_icon(icon_cx, icon_cy, 9.0f, active_perk->kind);

        // Timer bar to the left of the icon.
        constexpr float kBarW = 90.0f;
        constexpr float kBarH = 8.0f;
        const float bar_x = icon_cx - 14.0f - kBarW;
        const float bar_y = icon_cy - 4.0f;
        render::set_color({0.20f, 0.20f, 0.30f, 1.0f});
        render::draw_quad(bar_x, bar_y, kBarW, kBarH);
        render::set_color(gameplay::perk_color(active_perk->kind));
        render::draw_quad(bar_x, bar_y, kBarW * frac, kBarH);
    }
}

}  // namespace ui
