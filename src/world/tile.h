#pragma once

#include <cstdint>

namespace world {

// Maze grid geometry. These are referenced everywhere — keep them here as
// the single source of truth.
inline constexpr int kCols = 28;
inline constexpr int kRows = 31;
inline constexpr int kTileSize = 24;
inline constexpr int kPlayAreaWidth = kCols * kTileSize;   // 672
inline constexpr int kPlayAreaHeight = kRows * kTileSize;  // 744

// Per-tile classification after parsing. The on-disk ASCII has more glyphs
// (P, B, I, N, C) but those decay into Empty + a separate Spawns record.
enum class TileType : std::uint8_t {
    Empty,        // walkable, no collectible
    Wall,         // blocks Pac and ghosts
    Dot,          // walkable, +10 pts on pickup
    PowerPellet,  // walkable, +50 pts, frightens ghosts
    GhostDoor,    // ghosts can pass; Pac cannot
};

// Coordinate conversions. Tile (col, row) → top-left corner pixel of that
// tile cell. Px is the inverse, with truncation toward 0 (use only on
// non-negative inputs — out-of-bounds is the caller's problem).
inline int tile_to_px(int tile_xy) {
    return tile_xy * kTileSize;
}
inline int px_to_tile(int px) {
    return px / kTileSize;
}
inline int tile_center_px(int tile_xy) {
    return tile_xy * kTileSize + kTileSize / 2;
}

// Bounds-check helper for callers iterating outside their own grid (ghost AI,
// renderers checking neighbors, etc.).
inline bool in_bounds(int col, int row) {
    return col >= 0 && col < kCols && row >= 0 && row < kRows;
}

}  // namespace world
