#pragma once

#include "gameplay/ghost.h"
#include "gameplay/pacman.h"

namespace gameplay {

// Pac↔ghost overlap test in continuous pixel coordinates.
//
// Why not just `pac.col == ghost.col && pac.row == ghost.row`? Because two
// actors moving toward each other on the same row can pass through the
// boundary between two tiles within the same frame: their `(col, row)`
// pair never coincides even though their sprites visually overlap for
// several frames. Continuous-distance is the only correct test.
bool pac_ghost_overlap(const Pacman& p, const Ghost& g);

}  // namespace gameplay
