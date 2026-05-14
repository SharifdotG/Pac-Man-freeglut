#pragma once

#include "render/primitives.h"

namespace gameplay {

// Eat-streak multiplier. Eating a dot within `kComboWindow` seconds of
// the previous eat increments the chain count; otherwise it resets to 1.
// Score multiplier ramps from ×1.0 (chain==1) to ×3.0 (chain==21+),
// linear at +0.1 per chain link.
//
// Floating popups ("+10", "+22", "+200") are drawn near Pac for ~0.6 s,
// drifting up and fading. Implemented as a small ring buffer below to
// keep allocation off the per-frame path.
struct Combo {
    int chain = 0;
    float seconds_since_last_eat = 1000.0f;
};

constexpr float kComboWindow = 0.40f;
constexpr float kComboPerLink = 0.10f;
constexpr float kComboMax = 3.00f;
constexpr int kComboCapForMax = 21;

inline float combo_multiplier(const Combo& c) {
    if (c.chain <= 1)
        return 1.0f;
    const float m = 1.0f + kComboPerLink * static_cast<float>(c.chain - 1);
    return (m > kComboMax) ? kComboMax : m;
}

inline void combo_tick(Combo& c, float dt) {
    c.seconds_since_last_eat += dt;
    if (c.seconds_since_last_eat > kComboWindow && c.chain != 0) {
        c.chain = 0;  // expired streak
    }
}

inline void combo_on_eat(Combo& c) {
    if (c.seconds_since_last_eat <= kComboWindow) {
        ++c.chain;
    } else {
        c.chain = 1;
    }
    c.seconds_since_last_eat = 0.0f;
}

// === Floating-text popups =================================================
// Used by Game::update to spawn "+10 ×1.4" style readouts; rendered after
// the world but before the HUD so they sit on top of the maze.

void popup_emit(const char* text, float x, float y, render::Color color);
void popups_update(float dt);
void popups_render();
void popups_clear();

}  // namespace gameplay
