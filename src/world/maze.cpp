#include "world/maze.h"

namespace world {

int Maze::eat_at(int col, int row) {
    if (!in_bounds(col, row)) {
        return 0;
    }
    const int idx = index(col, row);
    switch (m_tiles[idx]) {
        case TileType::Dot:
            m_tiles[idx] = TileType::Empty;
            --m_dots_remaining;
            return 10;
        case TileType::PowerPellet:
            m_tiles[idx] = TileType::Empty;
            --m_dots_remaining;
            return 50;
        default:
            return 0;
    }
}

void Maze::recount_dots() {
    int n = 0;
    for (TileType t : m_tiles) {
        if (t == TileType::Dot || t == TileType::PowerPellet) {
            ++n;
        }
    }
    m_dots_remaining = n;
}

}  // namespace world
