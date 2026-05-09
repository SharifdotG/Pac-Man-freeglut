#pragma once

#include "gameplay/ghost.h"
#include "gameplay/modes.h"
#include "gameplay/pacman.h"
#include "util/direction.h"
#include "world/maze.h"

namespace gameplay {

// Decide which direction the ghost should head from its CURRENT tile.
// Called from `ghost_update` every time the ghost crosses into a new
// tile — never mid-tile.
//
// Behaviour by mode:
//   Chase      — kind-specific target tile (Blinky direct, Pinky 4-ahead,
//                Inky vector-doubled-from-Blinky, Clyde flee-when-near).
//   Scatter    — kind-specific home corner.
//   Frightened — RANDOM walkable direction, ignoring the no-reverse rule
//                only when there are zero forward options.
//   Eaten      — fixed target = ghost-house corridor (13, 11), so the
//                same greedy-nearest-neighbour code routes the eyes home.
//
// `blinky` is needed by Inky's chase target; for Blinky's own update the
// caller may pass `g` itself — Blinky's branch ignores `blinky`.
util::Direction ai_pick_next_direction(const Ghost& g,
                                       const world::Maze& m,
                                       const Pacman& pac,
                                       const Ghost& blinky,
                                       WaveMode wave_mode);

}  // namespace gameplay
