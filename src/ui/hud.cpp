#include "ui/hud.h"

#include <GL/freeglut.h>

#include <cmath>
#include <cstdio>

#include "render/primitives.h"
#include "render/text.h"
#include "world/tile.h"

namespace ui {

namespace {

// HUD lives in the bottom 80 px of the window: world y in [744, 824].
constexpr float kHudTopY = static_cast<float>(world::kPlayAreaHeight);  // 744

constexpr render::Color kSeparatorColor = {0.20f, 0.20f, 0.40f, 1.0f};
constexpr render::Color kLabelColor = {0.95f, 0.95f, 0.62f, 1.0f};
constexpr render::Color kValueColor = {1.00f, 1.00f, 1.00f, 1.0f};
constexpr render::Color kPacColor = {1.00f, 0.92f, 0.16f, 1.0f};

void draw_life_icon(float cx, float cy) {
    constexpr float kRadius = 8.0f;
    constexpr float kMouthHalf = 0.45f;
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

void render_hud(const gameplay::Score& s) {
    render::set_color(kSeparatorColor);
    render::draw_line(8.0f,
                      kHudTopY + 2.0f,
                      static_cast<float>(world::kPlayAreaWidth) - 8.0f,
                      kHudTopY + 2.0f,
                      1.5f);

    char buf[32];

    const float left_x = 24.0f;
    render::set_color(kLabelColor);
    render::draw_text(left_x, kHudTopY + 30.0f, "SCORE:");
    render::set_color(kValueColor);
    std::snprintf(buf, sizeof(buf), "%d", s.points());
    render::draw_text(left_x + 82.0f, kHudTopY + 30.0f, buf);

    const float center_x = static_cast<float>(world::kPlayAreaWidth) * 0.5f;
    render::set_color(kLabelColor);
    render::draw_text_centered(center_x, kHudTopY + 30.0f, "HI:");
    render::set_color(kValueColor);
    std::snprintf(buf, sizeof(buf), "%d", s.hi_score());
    render::draw_text_centered(center_x + 40.0f, kHudTopY + 30.0f, buf);

    std::snprintf(buf, sizeof(buf), "LEVEL %d", s.level());
    const int level_w = render::text_width_px(buf);
    const float level_x =
        static_cast<float>(world::kPlayAreaWidth) - 24.0f - static_cast<float>(level_w) - 80.0f;
    render::set_color(kLabelColor);
    render::draw_text(level_x, kHudTopY + 30.0f, buf);

    render::set_color(kLabelColor);
    render::draw_text(level_x, kHudTopY + 62.0f, "LIVES:");
    for (int i = 0; i < s.lives(); ++i) {
        const float cx = level_x + 66.0f + static_cast<float>(i) * 22.0f;
        const float cy = kHudTopY + 70.0f;
        draw_life_icon(cx, cy);
    }
}

}  // namespace ui
