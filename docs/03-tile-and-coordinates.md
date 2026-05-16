# 03 — Tile & Coordinate System

The whole game is a **28 × 31 tile grid** with **24-pixel tiles** living
inside a fixed **672 × 824** window. This file explains the math, why each
constant is what it is, and how to convert between coordinate spaces.

Authoritative source: [src/world/tile.h](../src/world/tile.h) and
[src/render/gl_init.h](../src/render/gl_init.h). Constants below mirror
those headers.

## Window vs play area

| Region | Pixels | Purpose |
| --- | --- | --- |
| Window | 672 × 824 | The whole `glutCreateWindow` surface |
| Play area | 672 × 744 | 28 × 31 tile grid, top-aligned |
| HUD strip | 672 × 80 | Bottom of the window; score, lives, level |

The play area's height is **31 rows × 24 px = 744**. The HUD takes the
remaining 80 px at the bottom.

## Why these numbers?

- **24-px tiles** — large enough to draw a recognisable Pac-Man and ghost
  inside one tile, small enough that 28 × 31 fits on a typical 1080p
  display without scaling.
- **28 × 31 grid** — matches the canonical arcade layout. Hard requirement
  for the level file format; do not change.
- **80-px HUD** — three text rows at 8-px bitmap font × 3 line height,
  plus a few pixels of margin. Exact layout in
  [09-ui-and-menus.md](09-ui-and-menus.md) (added in P4 / P8).

## Coordinate systems

### Pixel space (rendering)

- `gluOrtho2D(0, 672, 824, 0)` ⇒ origin **top-left**, x-right, y-down.
  This matches the way we think about screen coordinates and makes
  `tile_to_px(col, row) = (col * 24, row * 24)` the natural form.
- All `glVertex2f` calls are in pixels.

### Tile space (gameplay)

- `(col, row)` integers, both 0-indexed.
- `(0, 0)` is the top-left tile; `(27, 30)` is the bottom-right.
- Pac-Man and ghosts have a **continuous** position in pixels but their
  **logical** position (used for AI target tiles, dot pickup, etc.) is
  the tile their centre is currently in.

### Conversions

[src/world/tile.h](../src/world/tile.h):

```cpp
inline int tile_to_px(int xy)        { return xy * kTileSize; }
inline int px_to_tile(int px)        { return px / kTileSize; }
inline int tile_center_px(int xy)    { return xy * kTileSize + kTileSize / 2; }
```

`px_to_tile` truncates toward 0 — only call it on non-negative inputs. Tunnel
wrap (P3) handles the off-grid case explicitly.

## Tile types

See [src/world/tile.h](../src/world/tile.h) for the canonical enum:

```cpp
enum class TileType : std::uint8_t {
    Empty, Wall, Dot, PowerPellet, GhostDoor,
};
```

There is no `TileType::Tunnel`. Tunnel cells are just `Empty` cells in a
specific row; the wrap is handled by the actor (P3) when its centre crosses
the screen edge, not by the tile.

## Wall rendering

Walls are not drawn as solid blocks — that would look blocky and
un-arcade-like. Instead, [src/core/game.cpp](../src/core/game.cpp) uses
**edge-aware outlining**: for each `Wall` tile, draw a thin blue line
segment along each of its four sides **only if** the neighbouring tile on
that side is **not** a wall.

```cpp
if (!m.is_wall(col, row - 1)) draw_line(top_edge);
if (!m.is_wall(col, row + 1)) draw_line(bottom_edge);
if (!m.is_wall(col - 1, row)) draw_line(left_edge);
if (!m.is_wall(col + 1, row)) draw_line(right_edge);
```

`is_wall` treats out-of-bounds neighbours as **not** walls, so the
outermost wall tiles draw their outer edges and you get a closed perimeter
outline around the whole maze.

The result is the iconic Pac-Man "blue line" maze look. No textures are
needed for this — the entire maze is drawn from `glBegin(GL_QUADS)` line
segments.

## Common pitfalls

- **Off-by-one row count.** `kRows = 31`, indices `0..30`. The bottom row
  index is `30`, not `31`.
- **Y is down.** A common reflex from math class is y-up. `glVertex2f(0, 0)`
  is **top-left**, not bottom-left.
- **`px_to_tile` and negatives.** Don't call it with `px < 0`. Tunnel wrap
  needs explicit handling.
- **Line thickness is in pixels.** `draw_line(..., thickness=2.5)` produces
  a 2.5-px-wide quad. The choice of 2.5 (not 2) is deliberate — at 24-px
  tiles, a 2-px line looks a hair thin and a 3-px line looks chunky.
