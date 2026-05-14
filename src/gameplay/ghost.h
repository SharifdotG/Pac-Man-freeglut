#pragma once

#include "gameplay/modes.h"
#include "gameplay/pacman.h"
#include "util/direction.h"
#include "world/maze.h"

namespace gameplay {

enum class GhostKind : unsigned char {
    Blinky,  // red    — direct chaser
    Pinky,   // pink   — ambushes 4 tiles ahead of Pac
    Inky,    // cyan   — flanks via Blinky-pivot vector
    Clyde,   // orange — chases when far, retreats when close
};

enum class GhostMode : unsigned char {
    Chase,       // hunt Pac via the kind-specific target tile
    Scatter,     // wander toward home corner
    Frightened,  // blue, random direction at intersections; eatable
    Eaten,       // eyes-only, racing back to the ghost-house entrance
    InHouse,     // bobbing inside the house, waiting on a release timer
};

// Every per-ghost piece of state lives here. Ghost AI mutates `dir` /
// `mode_timer`; the game loop manages mode transitions and `pending_reverse`.
struct Ghost {
    GhostKind kind = GhostKind::Blinky;

    // (col, row, offset) movement model — same as Pacman so the tunnel
    // wrap and collision math are symmetric.
    int col = 0;
    int row = 0;
    float offset = 0.0f;
    util::Direction dir = util::Direction::Left;

    GhostMode mode = GhostMode::Scatter;
    float speed_tiles_sec = 5.5f;  // base; multiplied by mode at render-tick

    // Generic mode timer. Counts UP for InHouse (release-at gating) and
    // DOWN for Frightened (until 0 → revert to wave mode). Unused in the
    // other modes.
    float mode_timer = 0.0f;

    // Seconds (relative to game start) at which an InHouse ghost should
    // emerge through the door. Per-kind preset in ghost_init().
    float house_release_at = 0.0f;

    // Set by the game loop on a wave-mode change or power-pellet pickup.
    // The next ghost_update applies a mid-tile reversal and clears it.
    bool pending_reverse = false;
};

void ghost_init(Ghost& g, GhostKind k, int spawn_col, int spawn_row);

// One fixed-step tick. `blinky` is needed because Inky's chase target
// depends on Blinky's tile. For the Blinky update itself, callers may
// pass `g` as `blinky` — Blinky's own AI ignores that argument.
void ghost_update(Ghost& g,
                  float dt,
                  const world::Maze& m,
                  const Pacman& pac,
                  const Ghost& blinky,
                  WaveMode wave_mode);

void ghost_render(const Ghost& g);

float ghost_x_px(const Ghost& g);
float ghost_y_px(const Ghost& g);

}  // namespace gameplay
