#pragma once

#include "util/direction.h"
#include "world/maze.h"

namespace gameplay {

// Pac's position is **(col, row, offset)** where offset ∈ [0, 1) is the
// fractional progress from (col, row)'s tile centre toward (col+dx,
// row+dy). When offset hits 1.0 we cross into the next tile and reset
// offset to 0.
//
// Why this model rather than continuous (x, y) floats?
//  - Tile-aligned by construction: turns can only happen when offset==0,
//    which is exactly the "decision point" classic Pac-Man uses.
//  - Tunnel wrap is trivial: just wrap col when it leaves [0, kCols).
//  - No accumulating float drift across thousands of frames.
struct Pacman {
    int col = 0;
    int row = 0;
    float offset = 0.0f;

    util::Direction dir = util::Direction::None;       // current direction of motion
    util::Direction buffered = util::Direction::None;  // last input — applied at next tile boundary

    float anim_time = 0.0f;        // accumulated seconds while moving — drives the mouth chomp
    float speed_tiles_sec = 6.5f;  // tiles/sec; tunable per level later
    bool alive = true;
};

// Place Pac at the given tile spawn, facing nothing (idle). Pressing any
// direction key kicks him into motion.
void pacman_init(Pacman& p, int spawn_col, int spawn_row);

// Fixed-timestep update. Pulls input::state().wanted into `buffered` and
// runs the movement state machine.
void pacman_update(Pacman& p, float dt, const world::Maze& m);

// Render Pac at his interpolated continuous position. P3 draws him
// procedurally as a yellow circle with an animated mouth wedge — sprite
// atlas lands later.
void pacman_render(const Pacman& p);

// Continuous render position of Pac's centre, accounting for tunnel wrap
// (so the sprite doesn't spend half a tile off-screen during the warp).
float pacman_x_px(const Pacman& p);
float pacman_y_px(const Pacman& p);

}  // namespace gameplay
