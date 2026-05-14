#include "gameplay/ghost_ai.h"

#include <cstdlib>  // std::rand — OK here; gameplay rng is not security-critical

#include "world/pathfind.h"
#include "world/tile.h"

namespace gameplay {

using util::Direction;

namespace {

// Ghost-house corridor entrance. Eaten ghosts target this tile; ghost.cpp
// reads the same constant when handling release / re-enter events.
constexpr int kHouseEntryCol = 13;
constexpr int kHouseEntryRow = 11;

bool walkable_with_wrap(const world::Maze& m, int col, int row) {
    if (col < 0)
        col += world::kCols;
    else if (col >= world::kCols)
        col -= world::kCols;
    return m.walkable_for_ghost(col, row);
}

struct TargetTile {
    int col;
    int row;
};

// Each ghost's "home corner" used when in Scatter. These are off-grid on
// purpose (the canonical arcade values) — the ghost never reaches them
// and ends up patrolling near the corner instead.
TargetTile scatter_target(GhostKind k) {
    switch (k) {
        case GhostKind::Blinky:
            return {25, -3};  // top-right
        case GhostKind::Pinky:
            return {2, -3};  // top-left
        case GhostKind::Inky:
            return {27, 31};  // bottom-right
        case GhostKind::Clyde:
            return {0, 31};  // bottom-left
    }
    return {0, 0};
}

// Per-personality chase target. The four expressions below ARE the four
// classic personalities — the rest of the AI is identical between ghosts.
TargetTile chase_target(const Ghost& g, const Pacman& pac, const Ghost& blinky) {
    switch (g.kind) {
        case GhostKind::Blinky:
            // Direct chase.
            return {pac.col, pac.row};

        case GhostKind::Pinky:
            // 4 tiles ahead of Pac in his facing direction. (We deliberately
            // omit the original arcade's "Up overflow" bug — it's a bug, not
            // a feature, and players never knew about it.)
            return {pac.col + 4 * util::dx(pac.dir), pac.row + 4 * util::dy(pac.dir)};

        case GhostKind::Inky: {
            // Pivot tile = 2 ahead of Pac. Vector from Blinky to that pivot,
            // doubled, gives Inky's target. Net effect: Inky tries to flank
            // Pac on the side opposite from Blinky.
            const int piv_col = pac.col + 2 * util::dx(pac.dir);
            const int piv_row = pac.row + 2 * util::dy(pac.dir);
            const int dc = piv_col - blinky.col;
            const int dr = piv_row - blinky.row;
            return {piv_col + dc, piv_row + dr};
        }

        case GhostKind::Clyde: {
            // Direct chase if > 8 tiles from Pac, else flee toward home corner.
            // Produces the wandering "Clyde keeps almost catching you and
            // then losing interest" behaviour.
            const int dc = g.col - pac.col;
            const int dr = g.row - pac.row;
            if (dc * dc + dr * dr > 64)
                return {pac.col, pac.row};
            return scatter_target(GhostKind::Clyde);
        }
    }
    return {pac.col, pac.row};
}

TargetTile target_for(const Ghost& g, const Pacman& pac, const Ghost& blinky, WaveMode wave_mode) {
    if (g.mode == GhostMode::Eaten) {
        return {kHouseEntryCol, kHouseEntryRow};
    }
    if (g.mode == GhostMode::Frightened) {
        // Unused — frightened picks randomly. Return something harmless.
        return {pac.col, pac.row};
    }
    // Chase / Scatter share the targeting tree below; the ghost's *mode*
    // tells us whether to use the chase target or the scatter corner.
    if (wave_mode == WaveMode::Scatter || g.mode == GhostMode::Scatter) {
        return scatter_target(g.kind);
    }
    return chase_target(g, pac, blinky);
}

// Greedy: enumerate Up,Left,Down,Right; reject reverse + walls; pick
// the candidate that minimises squared Euclidean distance to `target`;
// strict `<` tie-break preserves the Up>Left>Down>Right bias.
Direction greedy_pick(const Ghost& g, const world::Maze& m, TargetTile target) {
    const Direction reverse = util::opposite(g.dir);
    constexpr Direction kCandidates[4] = {
        Direction::Up,
        Direction::Left,
        Direction::Down,
        Direction::Right,
    };

    Direction best = Direction::None;
    int best_dist = 0;
    for (Direction d : kCandidates) {
        if (d == reverse)
            continue;
        const int nc = g.col + util::dx(d);
        const int nr = g.row + util::dy(d);
        if (!walkable_with_wrap(m, nc, nr))
            continue;

        int eval_col = nc;
        if (eval_col < 0)
            eval_col += world::kCols;
        else if (eval_col >= world::kCols)
            eval_col -= world::kCols;

        const int dist_sq = world::distance_squared(eval_col, nr, target.col, target.row);
        if (best == Direction::None || dist_sq < best_dist) {
            best = d;
            best_dist = dist_sq;
        }
    }
    if (best != Direction::None)
        return best;

    // Dead-end: allow reverse.
    if (reverse != Direction::None &&
        walkable_with_wrap(m, g.col + util::dx(reverse), g.row + util::dy(reverse))) {
        return reverse;
    }
    return Direction::None;
}

// Pick a uniformly-random walkable forward direction. Frightened ghosts
// use this instead of greedy_pick.
Direction frightened_pick(const Ghost& g, const world::Maze& m) {
    const Direction reverse = util::opposite(g.dir);
    constexpr Direction kCandidates[4] = {
        Direction::Up,
        Direction::Left,
        Direction::Down,
        Direction::Right,
    };

    Direction options[4];
    int n = 0;
    for (Direction d : kCandidates) {
        if (d == reverse)
            continue;
        if (!walkable_with_wrap(m, g.col + util::dx(d), g.row + util::dy(d)))
            continue;
        options[n++] = d;
    }
    if (n > 0) {
        return options[std::rand() % n];
    }
    if (reverse != Direction::None &&
        walkable_with_wrap(m, g.col + util::dx(reverse), g.row + util::dy(reverse))) {
        return reverse;
    }
    return Direction::None;
}

}  // namespace

Direction ai_pick_next_direction(const Ghost& g,
                                 const world::Maze& m,
                                 const Pacman& pac,
                                 const Ghost& blinky,
                                 WaveMode wave_mode) {
    if (g.mode == GhostMode::Frightened) {
        return frightened_pick(g, m);
    }
    return greedy_pick(g, m, target_for(g, pac, blinky, wave_mode));
}

}  // namespace gameplay
