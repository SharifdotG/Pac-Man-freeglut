#include "gameplay/ghost.h"

#include <GL/freeglut.h>

#include <cmath>

#include "gameplay/ghost_ai.h"
#include "render/primitives.h"
#include "world/tile.h"

namespace gameplay {

using util::Direction;

namespace {

constexpr float kGhostRadius = 11.0f;

// Ghost-house entrance — corridor tile directly above the door. Used as
// the spawn-after-release point and as the target for Eaten ghosts heading
// home. Kept in sync with the shape of level_01.
constexpr int kHouseEntryCol = 13;
constexpr int kHouseEntryRow = 11;

// Mode-derived speed multipliers. Pac runs at 6.5 t/s; Frightened ghosts
// are slower than Pac (so the player can catch them), Eaten ghosts much
// faster than the base (so the eyes return home in a sensible time).
float speed_multiplier(GhostMode m) {
    switch (m) {
        case GhostMode::Frightened:
            return 0.50f;
        case GhostMode::Eaten:
            return 1.80f;
        default:
            return 1.00f;
    }
}

render::Color body_color(GhostKind k) {
    switch (k) {
        case GhostKind::Blinky:
            return {1.00f, 0.10f, 0.10f, 1.0f};
        case GhostKind::Pinky:
            return {1.00f, 0.72f, 0.85f, 1.0f};
        case GhostKind::Inky:
            return {0.10f, 0.85f, 1.00f, 1.0f};
        case GhostKind::Clyde:
            return {1.00f, 0.62f, 0.18f, 1.0f};
    }
    return {1, 1, 1, 1};
}

bool walkable_with_wrap(const world::Maze& m, int col, int row) {
    if (col < 0)
        col += world::kCols;
    else if (col >= world::kCols)
        col -= world::kCols;
    return m.walkable_for_ghost(col, row);
}

// Mid-tile reversal. Mirrors Pac's U-turn math: shift into the next tile
// the ghost was heading toward, invert the offset, flip dir.
void apply_pending_reverse(Ghost& g) {
    if (!g.pending_reverse)
        return;
    if (g.dir == Direction::None) {
        g.pending_reverse = false;
        return;
    }
    int nc = g.col + util::dx(g.dir);
    int nr = g.row + util::dy(g.dir);
    if (nc < 0)
        nc += world::kCols;
    else if (nc >= world::kCols)
        nc -= world::kCols;
    g.col = nc;
    g.row = nr;
    g.offset = 1.0f - g.offset;
    g.dir = util::opposite(g.dir);
    g.pending_reverse = false;
}

}  // namespace

void ghost_init(Ghost& g, GhostKind k, int spawn_col, int spawn_row) {
    g = Ghost{};
    g.kind = k;
    g.col = spawn_col;
    g.row = spawn_row;
    g.dir = Direction::Left;

    switch (k) {
        case GhostKind::Blinky:
            // Blinky always starts above the house, ready to chase. Override
            // the level spawn (which puts him inside the house with the
            // others). When P11 polish adds a real "walk out" animation,
            // this hard-coded position can come from the level instead.
            g.col = kHouseEntryCol;
            g.row = kHouseEntryRow;
            g.mode = GhostMode::Scatter;
            g.house_release_at = 0.0f;
            break;
        case GhostKind::Pinky:
            g.mode = GhostMode::InHouse;
            g.house_release_at = 1.0f;
            break;
        case GhostKind::Inky:
            g.mode = GhostMode::InHouse;
            g.house_release_at = 5.0f;
            break;
        case GhostKind::Clyde:
            g.mode = GhostMode::InHouse;
            g.house_release_at = 14.0f;
            break;
    }
}

void ghost_update(Ghost& g,
                  float dt,
                  const world::Maze& m,
                  const Pacman& pac,
                  const Ghost& blinky,
                  WaveMode wave_mode) {
    // === InHouse: sit at spawn until the release timer fires, then teleport
    // to the corridor entrance and switch to the current global wave mode.
    if (g.mode == GhostMode::InHouse) {
        g.mode_timer += dt;
        if (g.mode_timer >= g.house_release_at) {
            g.col = kHouseEntryCol;
            g.row = kHouseEntryRow;
            g.offset = 0.0f;
            g.dir = Direction::Left;
            g.mode = (wave_mode == WaveMode::Scatter) ? GhostMode::Scatter : GhostMode::Chase;
            g.mode_timer = 0.0f;
            g.pending_reverse = false;
        }
        return;
    }

    // === Frightened: tick the countdown; revert to wave mode at zero.
    if (g.mode == GhostMode::Frightened) {
        g.mode_timer -= dt;
        if (g.mode_timer <= 0.0f) {
            g.mode = (wave_mode == WaveMode::Scatter) ? GhostMode::Scatter : GhostMode::Chase;
            g.mode_timer = 0.0f;
            // Note: no reverse on frightened-end (matches arcade behaviour).
        }
    }

    apply_pending_reverse(g);

    // === Movement loop with mode-scaled speed.
    const float speed = g.speed_tiles_sec * speed_multiplier(g.mode);
    float advance = speed * dt;

    while (advance > 0.0f) {
        const float to_next_tile = 1.0f - g.offset;
        if (advance < to_next_tile) {
            g.offset += advance;
            advance = 0.0f;
            break;
        }

        // Cross into the next tile.
        advance -= to_next_tile;
        g.col += util::dx(g.dir);
        g.row += util::dy(g.dir);
        g.offset = 0.0f;

        // Tunnel wrap.
        if (g.col < 0)
            g.col += world::kCols;
        else if (g.col >= world::kCols)
            g.col -= world::kCols;

        // Eaten ghost arrived at the house entrance? Snap back to the
        // current wave mode — visually "the eyes re-grow into a ghost".
        // The walking-down-into-the-house animation is a P11 polish item.
        if (g.mode == GhostMode::Eaten && g.col == kHouseEntryCol && g.row == kHouseEntryRow) {
            g.mode = (wave_mode == WaveMode::Scatter) ? GhostMode::Scatter : GhostMode::Chase;
            g.dir = Direction::Left;
            advance = 0.0f;
            break;
        }

        // Ask the AI which direction to head from this new tile.
        const Direction next = ai_pick_next_direction(g, m, pac, blinky, wave_mode);
        if (next == Direction::None) {
            advance = 0.0f;
            break;
        }
        g.dir = next;

        // Sanity bail on a corner pocket.
        if (!walkable_with_wrap(m, g.col + util::dx(g.dir), g.row + util::dy(g.dir))) {
            advance = 0.0f;
            break;
        }
    }
}

float ghost_x_px(const Ghost& g) {
    float x = static_cast<float>(world::tile_to_px(g.col)) + world::kTileSize * 0.5f +
              g.offset * static_cast<float>(world::kTileSize) * static_cast<float>(util::dx(g.dir));
    if (x < 0.0f)
        x += static_cast<float>(world::kPlayAreaWidth);
    else if (x >= static_cast<float>(world::kPlayAreaWidth))
        x -= static_cast<float>(world::kPlayAreaWidth);
    return x;
}

float ghost_y_px(const Ghost& g) {
    return static_cast<float>(world::tile_to_px(g.row)) + world::kTileSize * 0.5f +
           g.offset * static_cast<float>(world::kTileSize) * static_cast<float>(util::dy(g.dir));
}

void ghost_render(const Ghost& g) {
    const float cx = ghost_x_px(g);
    const float cy = ghost_y_px(g);

    // Body is suppressed for Eaten (eyes-only), or recoloured for
    // Frightened (deep blue with a near-end white flash).
    const bool draw_body = (g.mode != GhostMode::Eaten);
    const bool show_pupils = (g.mode != GhostMode::Frightened);

    render::Color body = body_color(g.kind);
    if (g.mode == GhostMode::Frightened) {
        // White-flash in the last 2 seconds, ~3 Hz alternation.
        const bool flash = g.mode_timer < 2.0f && std::fmod(g.mode_timer, 0.30f) < 0.15f;
        body = flash ? render::Color{1.0f, 1.0f, 1.0f, 1.0f}
                     : render::Color{0.10f, 0.10f, 0.85f, 1.0f};
    }

    if (draw_body) {
        render::set_color(body);

        constexpr int kArcSegs = 18;
        constexpr float kPi = 3.14159265358979323846f;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= kArcSegs; ++i) {
            const float t = -kPi + (kPi * static_cast<float>(i) / static_cast<float>(kArcSegs));
            glVertex2f(cx + std::cos(t) * kGhostRadius, cy + std::sin(t) * kGhostRadius);
        }
        glEnd();

        render::draw_quad(cx - kGhostRadius, cy, kGhostRadius * 2.0f, kGhostRadius);
    }

    // Eyes — always rendered (Eaten ghosts are JUST eyes; Frightened have
    // simplified white circles without coloured pupils).
    constexpr float kEyeOffsetX = 4.0f;
    constexpr float kEyeY = -3.0f;
    constexpr float kEyeRadius = 3.0f;
    constexpr float kPupilRadius = 1.6f;
    constexpr float kPupilShift = 1.5f;

    render::set_color({1.0f, 1.0f, 1.0f, 1.0f});
    render::draw_filled_circle(cx - kEyeOffsetX, cy + kEyeY, kEyeRadius, 14);
    render::draw_filled_circle(cx + kEyeOffsetX, cy + kEyeY, kEyeRadius, 14);

    if (show_pupils) {
        const float dx_pupil = static_cast<float>(util::dx(g.dir)) * kPupilShift;
        const float dy_pupil = static_cast<float>(util::dy(g.dir)) * kPupilShift;
        render::set_color({0.10f, 0.10f, 0.55f, 1.0f});
        render::draw_filled_circle(
            cx - kEyeOffsetX + dx_pupil, cy + kEyeY + dy_pupil, kPupilRadius, 10);
        render::draw_filled_circle(
            cx + kEyeOffsetX + dx_pupil, cy + kEyeY + dy_pupil, kPupilRadius, 10);
    }
}

}  // namespace gameplay
