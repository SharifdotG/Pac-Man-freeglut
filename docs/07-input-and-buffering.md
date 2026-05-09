# 07 — Input & Direction Buffering

How the keyboard becomes Pac's motion. The two pieces are
[src/input/input.cpp](../src/input/input.cpp) (raw key events) and
[src/gameplay/pacman.cpp](../src/gameplay/pacman.cpp) (turn logic).

## Key bindings (P3)

| Key | Action |
| --- | --- |
| `↑` / `W` | Set wanted direction = Up |
| `↓` / `S` | Set wanted direction = Down |
| `←` / `A` | Set wanted direction = Left |
| `→` / `D` | Set wanted direction = Right |
| `Esc` / `Q` | Quit |

Pause (`P`) and fullscreen (`F11`) land in P8.

## What happens on a keypress

`input::on_keyboard_down` (printable keys) and `input::on_special_down`
(arrow keys / function keys) both feed a single shared field:

```cpp
input::state().wanted = util::Direction::Up;  // for example
```

Releasing the key does **not** clear `wanted`. The buffer is sticky:
once a direction is requested, Pac keeps trying to apply it until either
he succeeds (turns at the next intersection) or the user presses a
different direction key. This is exactly the behaviour of the original
arcade — a brief tap a few pixels before an intersection still takes.

## How Pac applies the buffer

[pacman.cpp](../src/gameplay/pacman.cpp) reads `input::state().wanted`
into `Pacman::buffered` once per fixed-step tick. The movement state
machine then checks the buffer at three points:

### 1. Idle → moving

When Pac is standing still (`dir == None`) and a direction is wanted, he
starts moving immediately if the neighbouring tile in that direction is
walkable. If it's a wall, the buffer just sits — pressing a key into a
wall does nothing visible until you turn toward something walkable.

### 2. At every tile centre

Each time Pac crosses into a new tile (`offset` reaches 1.0), the buffer
is consulted:

```cpp
if (buffered != None && buffered != dir
    && walkable(col + dx(buffered), row + dy(buffered))) {
    dir = buffered;
}
```

This is the **tap-ahead window**: tap left a few pixels before an
intersection, and Pac turns there as soon as he arrives.

### 3. U-turn (instant, mid-tile)

Pressing the **opposite** of the current direction triggers an
immediate, in-place reversal — no waiting for the next tile boundary.
The trick is to reframe Pac as already in the tile he was heading
toward, with offset inverted:

```cpp
col += dx(dir);       // jump into the next tile
offset = 1.0 - offset; // mirror the progress
dir = buffered;       // flip
```

Verify by inspecting the continuous render position: it doesn't change
across the U-turn — only the direction of motion does.

## Tile-locked turning, and why

The `(col, row, offset)` movement model means turns can happen **only at
tile centres** (offset == 0). This is faithful to the arcade: you cannot
turn into a corridor at random points along its length. The only
exception is the U-turn case above, which is allowed mid-tile because it
doesn't actually require entering new geometry — Pac is just reversing
along the corridor he's already in.

Trying to turn into a wall is silently ignored. Pac keeps moving in his
current direction, sliding along the wall as the player would expect.

## Tunnel wrap

The tunnel row (currently row 14) has Empty tiles at both edges of the
maze. When Pac walks off the left edge:

1. `col += dx(Left)` makes `col = -1`.
2. The wrap check normalises it: `col = kCols - 1` (the rightmost
   column).
3. Render position uses the same wrap so Pac visually appears on the
   opposite side without a jarring pop.

The wrap is column-only; rows never wrap.

## Speed

Pac moves at `6.5 tiles per second` in P3 — about 156 px/s. This is
faster than the arcade base speed but feels right at our 60 Hz update.
P7 will scale this up per-level for difficulty.

## What's NOT in P3

- No collision with ghosts (P5)
- No dot pickup or scoring (P4)
- No pause / fullscreen (P8)
- No animations beyond the mouth chomp (sprite atlas in a later phase)
