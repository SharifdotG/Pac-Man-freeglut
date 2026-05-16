# 11 — Level Format

Levels live in [assets/levels/](../assets/levels/) as plain ASCII text files.
The parser is [src/world/level_loader.cpp](../src/world/level_loader.cpp);
geometry constants are in [src/world/tile.h](../src/world/tile.h).

## Grid dimensions

- **28 columns × 31 rows.** Non-negotiable — these are baked into the
  renderer, the AI's neighbour-tile math, and the window size.
- One character = one tile (24 × 24 px on screen).

The loader is **lenient about trailing whitespace**: if a line is shorter
than 28 characters it gets right-padded with spaces (Empty tiles). Most
text editors strip trailing whitespace on save, so writing a line like
`"     #.##          ##.#"` (23 chars) is fine — it parses identically
to the same line padded out to 28.

Lines **longer** than 28 characters are an error.

## Character legend

| Char | Meaning | Tile after parse | Notes |
| --- | --- | --- | --- |
| `#` | Wall | `Wall` | Blocks Pac and ghosts; renderer outlines edges that face non-walls. |
| `.` | Dot | `Dot` | +10 pts on pickup. |
| `o` / `O` | Power pellet | `PowerPellet` | +50 pts; frightens ghosts (P6+). |
| ` ` (space) | Empty | `Empty` | Walkable, no collectible. |
| `\t` (tab) | Empty | `Empty` | Tabs aren't expanded — count as a single empty tile. Avoid. |
| `-` / `_` | Ghost-house door | `GhostDoor` | Ghosts can pass; Pac cannot. |
| `P` | Pac-Man spawn | `Empty` + spawn record | Exactly one per level. |
| `B` | Blinky spawn | `Empty` + spawn record | Exactly one per level. |
| `I` | Inky spawn | `Empty` + spawn record | Exactly one per level. |
| `N` | Pinky spawn (pi**N**ky) | `Empty` + spawn record | `P` is taken; `N` is the second consonant in piNky. |
| `C` | Clyde spawn | `Empty` + spawn record | Exactly one per level. |

Anything else fails the parse with a message like:

```text
[pacman:ERROR] level row 14 col 6: bad char '@' (0x40)
```

## Required structure

The loader rejects a level unless **all five** spawn markers appear exactly
once. There is currently no flood-fill reachability check — that lives in
the [add-level skill](../.claude/skills/add-level.md), which you should run
mentally when authoring a new layout.

## Authoring tips

- **Mirror left-to-right.** A symmetric maze looks polished and the original
  arcade map is symmetric.
- **Power pellets in the corners.** Convention is 4 pellets, one per
  quadrant, near the corners.
- **Ghost house centred.** Place it around (col 11–16, row 12–16) with a
  ghost-house door (`-`) on its top edge.
- **Tunnel row.** Pick one row (the original uses row 14 — the ghost-house
  centreline) and leave its left- and right-most few cells as Empty so Pac
  can cross. The wrap logic (P3) reads from `Empty` tiles at columns -1 and
  28 and treats them as warps to the opposite side.
- **Aim for ~240 dots + 4 power pellets** — that's the canonical arcade
  total. The current `level_01.txt` has **241 dots + 4 power pellets =
  245 collectibles**. Slightly more or fewer is fine; the win condition
  reads "all dots gone," not a magic number.

## Example: the first row

The top row in `level_01.txt` is just the maze border:

```text
############################
```

…and the second is the first row of dots, with a wall column straight down
the middle:

```text
#............##............#
```

Note the `##` in the middle — that's a 2-tile wall column splitting the
maze visually. The dot count for that row is `12 + 12 = 24`.

## Current level files

The repo ships three **visually-distinct** level files under
[assets/levels/](../assets/levels/):

| File | Geometry | Collectibles |
| --- | --- | --- |
| `level_01.txt` | Canonical 28×31 maze | 241 dots + 4 power pellets |
| `level_02.txt` | Same outer shell as L1, but the upper-half wall blocks restructured to **3-wide** + **6-wide** bands instead of L1's 4-wide + 5-wide rhythm. New corridor angles in rows 2–4. | 241 + 4 |
| `level_03.txt` | L2's upper-half changes **plus** wider middle-band walls (rows 6–7 widened to a single 12-wide block with narrow side corridors) **plus** mirrored upper-style bands in rows 21–22. Tighter overall feel. | 241 + 4 |

All three keep the ghost house (rows 9–19) and tunnel (row 14) layout
identical so the existing AI works without changes. Difficulty is
**both** geometric (distinct mazes) **and** mechanical: ghost speed
× 1.08 / × 1.16 by level, Pac × 1.05 / × 1.10, frightened window
6 s → 5.1 s → 4.3 s.

## Adding a new level

See the [add-level](../.claude/skills/add-level.md) skill for the canonical
recipe. The short version:

1. Copy `level_01.txt` to `level_02.txt`.
2. Edit the geometry; preserve **exactly** one of each spawn marker.
3. Save and rebuild. The CMake POST_BUILD step copies the new file next to
   the exe automatically.
4. Wire the new level path into the `kLevelFiles` list in
   [src/core/game.cpp](../src/core/game.cpp) (added in P7).
