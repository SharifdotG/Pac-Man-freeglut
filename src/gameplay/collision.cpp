#include "gameplay/collision.h"

namespace gameplay {

namespace {
// Touch threshold in pixels. Pac and Blinky are each ~11 px in radius, so
// summed radii ≈ 22 px. We use a slightly conservative 14 px so the player
// has to actually run into Blinky rather than die from a near-miss; feels
// fair without being lenient.
constexpr float kTouchPx = 14.0f;
constexpr float kTouchPxSq = kTouchPx * kTouchPx;
}  // namespace

bool pac_ghost_overlap(const Pacman& p, const Ghost& g) {
    const float px = pacman_x_px(p);
    const float py = pacman_y_px(p);
    const float gx = ghost_x_px(g);
    const float gy = ghost_y_px(g);
    const float dx = px - gx;
    const float dy = py - gy;
    return (dx * dx + dy * dy) < kTouchPxSq;
}

}  // namespace gameplay
