---
name: add-level
description: Author a new ASCII maze level, validate it, register it, and verify it loads correctly in-game.
---

# add-level

Use this skill when you need to add a new level to the game (or modify an
existing one). Levels live in `assets/levels/levelNN.txt` as 28×31-character
ASCII grids.

## Character legend

See [docs/11-level-format.md](../../docs/11-level-format.md) for the canonical
spec. Quick reference:

| Char | Meaning |
|---|---|
| `#` | Wall |
| `.` | Dot (small pellet, +10 pts) |
| `o` | Power pellet (+50 pts, frightens ghosts) |
| ` ` | Empty space (no dot) |
| `-` | Ghost-house door |
| `P` | Pac-Man spawn (exactly one per level) |
| `B` | Blinky spawn |
| `I` | Inky spawn |
| `N` | Pinky spawn |
| `C` | Clyde spawn |
| `T` | Tunnel marker (optional, on the row that wraps) |

## Steps

1. **Pick the next available number.** Look at `assets/levels/`. If `level_01.txt`
   and `level_02.txt` exist, the next is `level_03.txt`.

2. **Author the grid** as plain text, **28 columns × 31 rows**, using the
   characters above. Easiest: copy an existing level and modify.

3. **Validate before saving:**
   - Exactly **28 columns** on every row (count carefully — trailing spaces
     count).
   - Exactly **31 rows** total.
   - Exactly **one `P`** (Pac spawn).
   - **Four ghost spawns**: one each of `B`, `I`, `N`, `C`.
   - **At least four `o`** (power pellets), conventionally in the corners.
   - **All dots reachable** from `P` via flood-fill through walkable tiles
     (anything that's not `#` or `-`). If a dot is unreachable, the player
     can't win.

4. **Save** as `assets/levels/level_NN.txt` with `NN` zero-padded.

5. **Register the level** in [src/core/game.cpp](../../src/core/game.cpp).
   Look for the `kLevelFiles` array (or equivalent) and append the new path.
   Increment the level count constant if there is one.

6. **Build and run:**

   ```bash
   cmake --build build -j
   ./build/bin/pacman.exe
   ```

   Play through to the new level (or use a debug shortcut if one exists, e.g.
   pressing `L` to skip levels — check current keybindings in
   [docs/07-input-and-buffering.md](../../docs/07-input-and-buffering.md)).

7. **Verify:**
   - Maze renders without missing wall tiles or visual glitches.
   - Pac and ghosts spawn at the right positions.
   - Dot count in the HUD matches the count in the file (`grep -c '\\.\\|o' …`).
   - You can clear the level (all dots eaten triggers LEVEL_COMPLETE).

8. **Document:** add a note in [docs/11-level-format.md](../../docs/11-level-format.md)
   if the new level introduces a layout idiom others should know about.

## Common pitfalls

- **Off-by-one columns:** an editor that strips trailing whitespace will turn
  ` ` (space) at end-of-row into nothing, breaking the 28-col invariant.
  Pad with explicit `#` or `.` rather than trailing spaces.
- **Disconnected ghost house:** the ghost-house door (`-`) must connect the
  enclosed ghost-spawn area to the rest of the maze.
- **Tunnel asymmetry:** if you use `T`, place it on both ends of the same row;
  the wrap logic assumes left-edge ↔ right-edge of the same row.
