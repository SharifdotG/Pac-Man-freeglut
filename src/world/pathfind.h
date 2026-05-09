#pragma once

namespace world {

// Squared Euclidean distance between two tile coordinates. Ghosts use this
// directly — the original Pac-Man arcade picks the candidate tile with the
// smallest *squared* Euclidean distance to its target (not Manhattan, and
// not raw Euclidean — sqrt is unnecessary because we only compare).
inline int distance_squared(int c1, int r1, int c2, int r2) {
    const int dc = c1 - c2;
    const int dr = r1 - r2;
    return dc * dc + dr * dr;
}

}  // namespace world
