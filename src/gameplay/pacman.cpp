#include "gameplay/pacman.h"

#include <GL/freeglut.h>

#include <cmath>

#include "input/input.h"
#include "render/primitives.h"
#include "world/tile.h"

namespace gameplay {

using util::Direction;

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;
constexpr float kPi = 3.14159265358979323846f;

// How many times per second the mouth opens-and-closes while Pac is moving.
constexpr float kChompHz = 6.0f;

// Sprite radius. Slightly under the tile half-size so Pac has visible
// breathing room in narrow corridors.
constexpr float kPacRadius = 11.0f;

// Yellow used by the original arcade — saturated, slightly green-shifted.
constexpr render::Color kPacColor = {1.0f, 0.92f, 0.16f, 1.0f};

bool collision_check_world(const world::Maze& m, int col, int row) {
    return !m.walkable_for_pac(col, row);
}

bool walkable_with_wrap(const world::Maze& m, int col, int row) {
    // Keep Pac inside the maze bounds on the row axis.
    if (row < 0 || row >= world::kRows)
        return false;

    // Tunnel wrap: any column off the grid maps to the opposite edge.
    if (col < 0)
        col += world::kCols;
    else if (col >= world::kCols)
        col -= world::kCols;

    return !collision_check_world(m, col, row);
}

// Center-angle (radians, screen coords with y-down) of Pac's mouth for each
// facing direction. When idle, default to Right so the sprite still has a
// recognisable mouth.
float facing_angle(Direction d) {
    switch (d) {
        case Direction::Up:
            return -kPi * 0.5f;
        case Direction::Down:
            return kPi * 0.5f;
        case Direction::Left:
            return kPi;
        case Direction::Right:
            return 0.0f;
        default:
            return 0.0f;  // idle → face right
    }
}

}  // namespace

void pacman_init(Pacman& p, int spawn_col, int spawn_row) {
    p = Pacman{};
    p.col = spawn_col;
    p.row = spawn_row;
}

void pacman_update(Pacman& p, float dt, const world::Maze& m) {
    if (!p.alive) {
        return;
    }

    // Pull the latest desired direction from the input buffer. We don't
    // clear it — the buffer persists across frames so a key tapped just
    // before an intersection still applies once Pac arrives there.
    p.buffered = input::state().wanted;

    // Instant U-turn: if the player presses the opposite of the current
    // direction, flip immediately mid-tile by moving Pac into the tile he
    // was heading toward and inverting the offset.
    if (p.dir != Direction::None && util::is_opposite(p.buffered, p.dir)) {
        int nc = p.col + util::dx(p.dir);
        int nr = p.row + util::dy(p.dir);
        if (nc < 0)
            nc += world::kCols;
        else if (nc >= world::kCols)
            nc -= world::kCols;
        p.col = nc;
        p.row = nr;
        p.offset = 1.0f - p.offset;
        p.dir = p.buffered;
    }

    // Try to start moving from a dead stop if the buffered direction is
    // walkable from the current tile.
    if (p.dir == Direction::None && p.buffered != Direction::None) {
        const int nc = p.col + util::dx(p.buffered);
        const int nr = p.row + util::dy(p.buffered);
        if (walkable_with_wrap(m, nc, nr)) {
            p.dir = p.buffered;
        }
    }

    if (p.dir == Direction::None) {
        // Standing still — freeze the mouth in the closed pose.
        p.anim_time = 0.0f;
        return;
    }

    // Advance the offset, possibly crossing one or more tile boundaries
    // this tick (the loop handles high speeds correctly even though our
    // current speed easily fits in one tile per tick).
    float advance = p.speed_tiles_sec * dt;
    while (advance > 0.0f && p.dir != Direction::None) {
        const float to_next_tile = 1.0f - p.offset;
        if (advance < to_next_tile) {
            p.offset += advance;
            advance = 0.0f;
            break;
        }

        // Cross into the next tile.
        advance -= to_next_tile;
        p.col += util::dx(p.dir);
        p.row += util::dy(p.dir);
        p.offset = 0.0f;

        // Tunnel wrap (cheap: only horizontal cols wrap, never rows).
        if (p.col < 0)
            p.col += world::kCols;
        else if (p.col >= world::kCols)
            p.col -= world::kCols;

        // At the new tile centre we have a turn opportunity. Apply the
        // buffered direction if it differs from the current one and is
        // walkable from here.
        if (p.buffered != Direction::None && p.buffered != p.dir) {
            const int nc = p.col + util::dx(p.buffered);
            const int nr = p.row + util::dy(p.buffered);
            if (walkable_with_wrap(m, nc, nr)) {
                p.dir = p.buffered;
            }
        }

        // After (potentially) turning, can we still continue forward? If
        // not, freeze at this tile centre.
        const int fc = p.col + util::dx(p.dir);
        const int fr = p.row + util::dy(p.dir);
        if (!walkable_with_wrap(m, fc, fr)) {
            p.dir = Direction::None;
            advance = 0.0f;
        }
    }

    p.anim_time += dt;
}

float pacman_x_px(const Pacman& p) {
    float x = static_cast<float>(world::tile_to_px(p.col)) + world::kTileSize * 0.5f +
              p.offset * static_cast<float>(world::kTileSize) * static_cast<float>(util::dx(p.dir));
    // Render-side tunnel smoothing: if we're rendering off-screen because
    // of an in-progress wrap (col=0, dir=Left, offset>0.5 puts us slightly
    // negative), pull the render position to the opposite side so Pac
    // doesn't pop in/out at the seam.
    if (x < 0.0f)
        x += static_cast<float>(world::kPlayAreaWidth);
    else if (x >= static_cast<float>(world::kPlayAreaWidth))
        x -= static_cast<float>(world::kPlayAreaWidth);
    return x;
}

float pacman_y_px(const Pacman& p) {
    return static_cast<float>(world::tile_to_px(p.row)) + world::kTileSize * 0.5f +
           p.offset * static_cast<float>(world::kTileSize) * static_cast<float>(util::dy(p.dir));
}

void pacman_render(const Pacman& p) {
    const float cx = pacman_x_px(p);
    const float cy = pacman_y_px(p);
    const float center = facing_angle(p.dir);

    // Mouth phase: oscillates [0..1..0] using a rectified sine. While
    // standing still anim_time is held at 0 so the mouth stays closed.
    const float mouth_open = 0.5f - 0.5f * std::cos(p.anim_time * kChompHz * kTwoPi);
    // Half-angle of the mouth wedge in radians. Maxes at ~25° (so the
    // full wedge is ~50°, which reads as a confident chomp).
    const float mouth_half = mouth_open * 0.45f;

    render::set_color(kPacColor);

    // Draw a filled circle that omits the mouth wedge. GL_TRIANGLE_FAN with
    // a centre vertex + ring of perimeter vertices, skipping the wedge
    // around `center`.
    constexpr int kSegments = 28;
    const float arc_start = center + mouth_half;
    const float arc_end = center + kTwoPi - mouth_half;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= kSegments; ++i) {
        const float t = arc_start + (arc_end - arc_start) * static_cast<float>(i) /
                                        static_cast<float>(kSegments);
        glVertex2f(cx + std::cos(t) * kPacRadius, cy + std::sin(t) * kPacRadius);
    }
    glEnd();
}

}  // namespace gameplay
