#include "ui/help.h"

#include <GL/freeglut.h>

#include <cmath>

#include "render/gl_init.h"
#include "render/primitives.h"
#include "render/text.h"

namespace ui {

namespace {

constexpr render::Color kHeaderColor = {1.00f, 0.92f, 0.16f, 1.0f};
constexpr render::Color kLabelColor = {0.95f, 0.95f, 0.62f, 1.0f};
constexpr render::Color kTextColor = {0.85f, 0.85f, 0.95f, 1.0f};
constexpr render::Color kFooterColor = {0.50f, 0.50f, 0.65f, 1.0f};

// Tiny ghost icon used in the ghost legend. Drawn directly here so we don't
// have to construct a full Ghost struct just for help text.
void draw_ghost_icon(float cx, float cy, render::Color body) {
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float r = 7.0f;
    render::set_color(body);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    constexpr int segs = 14;
    for (int i = 0; i <= segs; ++i) {
        const float t = -kPi + kPi * static_cast<float>(i) / static_cast<float>(segs);
        glVertex2f(cx + std::cos(t) * r, cy + std::sin(t) * r);
    }
    glEnd();
    render::draw_quad(cx - r, cy, r * 2.0f, r);

    // White eye highlights so the icons read as ghosts at a glance.
    render::set_color({1.0f, 1.0f, 1.0f, 1.0f});
    render::draw_filled_circle(cx - 2.5f, cy - 2.0f, 1.7f, 10);
    render::draw_filled_circle(cx + 2.5f, cy - 2.0f, 1.7f, 10);
}

}  // namespace

void render_help_screen() {
    const float cx = static_cast<float>(render::kWindowWidth) * 0.5f;

    render::set_color(kHeaderColor);
    render::draw_text_centered(cx, 90.0f, "HOW TO PLAY");
    // P12: accent line under the header for visual rhythm with other screens.
    render::set_color({0.95f, 0.72f, 0.10f, 1.0f});
    render::draw_quad(cx - 70.0f, 106.0f, 140.0f, 2.0f);

    // === Controls block
    render::set_color(kLabelColor);
    render::draw_text(120.0f, 170.0f, "CONTROLS");

    struct Row {
        const char* key;
        const char* action;
    };
    constexpr Row kRows[] = {
        {"Arrow keys / WASD", "Move Pac-Man"},
        {"P", "Pause / Resume"},
        {"F11", "Toggle fullscreen"},
        {"Enter", "Confirm menu selection"},
        {"Esc", "Back / cancel"},
        {"Q", "Quit"},
    };

    float y = 200.0f;
    for (const auto& r : kRows) {
        render::set_color(kLabelColor);
        render::draw_text(140.0f, y, r.key);
        render::set_color(kTextColor);
        render::draw_text(360.0f, y, r.action);
        y += 26.0f;
    }

    // === Ghost legend
    render::set_color(kLabelColor);
    render::draw_text(120.0f, y + 30.0f, "GHOSTS");
    y += 60.0f;

    struct GhostRow {
        const char* name;
        render::Color color;
        const char* trait;
    };
    const GhostRow kGhostRows[] = {
        {"Blinky", {1.0f, 0.10f, 0.10f, 1.0f}, "chases Pac directly"},
        {"Pinky", {1.0f, 0.72f, 0.85f, 1.0f}, "ambushes 4 tiles ahead"},
        {"Inky", {0.1f, 0.85f, 1.00f, 1.0f}, "flanks via Blinky's pivot"},
        {"Clyde", {1.0f, 0.62f, 0.18f, 1.0f}, "erratic; flees when close"},
    };

    for (const auto& g : kGhostRows) {
        draw_ghost_icon(150.0f, y - 6.0f, g.color);
        render::set_color(kLabelColor);
        render::draw_text(180.0f, y, g.name);
        render::set_color(kTextColor);
        render::draw_text(280.0f, y, g.trait);
        y += 28.0f;
    }

    // === Pellets / fruit hint
    render::set_color(kTextColor);
    render::draw_text_centered(cx, y + 40.0f, "Eat power pellets to make ghosts edible.");
    render::draw_text_centered(cx, y + 64.0f, "Catch fruit for a big bonus.");

    // === Footer
    render::set_color(kFooterColor);
    render::draw_text_centered(cx, 730.0f, "Press Esc or Enter to return");
}

}  // namespace ui
