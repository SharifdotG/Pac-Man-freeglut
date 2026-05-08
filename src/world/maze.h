#pragma once

#include <array>

#include "world/tile.h"

namespace world {

// Spawn tiles parsed from the level file. Pac and the four ghosts each have
// a unique entry character; level_loader writes these and clears the
// underlying tiles to Empty.
struct Spawns {
    int pac_col = -1;
    int pac_row = -1;
    int blinky_col = -1;
    int blinky_row = -1;
    int pinky_col = -1;
    int pinky_row = -1;
    int inky_col = -1;
    int inky_row = -1;
    int clyde_col = -1;
    int clyde_row = -1;

    bool complete() const {
        return pac_col >= 0 && pac_row >= 0 && blinky_col >= 0 && blinky_row >= 0 &&
               pinky_col >= 0 && pinky_row >= 0 && inky_col >= 0 && inky_row >= 0 &&
               clyde_col >= 0 && clyde_row >= 0;
    }
};

class Maze {
   public:
    Maze() {
        m_tiles.fill(TileType::Empty);
    }

    TileType at(int col, int row) const {
        return in_bounds(col, row) ? m_tiles[index(col, row)] : TileType::Wall;
    }

    void set(int col, int row, TileType t) {
        if (in_bounds(col, row)) {
            m_tiles[index(col, row)] = t;
        }
    }

    // Pac walks on Empty/Dot/PowerPellet. Walls and the ghost door block him.
    bool walkable_for_pac(int col, int row) const {
        const TileType t = at(col, row);
        return t == TileType::Empty || t == TileType::Dot || t == TileType::PowerPellet;
    }

    // Ghosts may also pass through the ghost-house door.
    bool walkable_for_ghost(int col, int row) const {
        return walkable_for_pac(col, row) || at(col, row) == TileType::GhostDoor;
    }

    // Convenience used by the renderer's edge-aware wall outline. Treats
    // out-of-bounds as "not a wall" so the outer perimeter gets its outer
    // edges drawn.
    bool is_wall(int col, int row) const {
        return in_bounds(col, row) && m_tiles[index(col, row)] == TileType::Wall;
    }

    // Eat the dot at (col,row) if there is one. No-op otherwise. Returns the
    // points scored (10 for Dot, 50 for PowerPellet, 0 for nothing).
    int eat_at(int col, int row);

    int dots_remaining() const {
        return m_dots_remaining;
    }

    // After populating tiles, call this once so future dots_remaining queries
    // are accurate. level_loader does this for you.
    void recount_dots();

    const Spawns& spawns() const {
        return m_spawns;
    }
    void set_spawns(const Spawns& s) {
        m_spawns = s;
    }

   private:
    static constexpr int index(int col, int row) {
        return row * kCols + col;
    }

    std::array<TileType, kCols * kRows> m_tiles{};
    Spawns m_spawns;
    int m_dots_remaining = 0;
};

}  // namespace world
