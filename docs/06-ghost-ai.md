# 06 — Ghost AI

How the four ghosts decide where to go, when to switch behaviours, and
how they react to power pellets.

Authoritative code:

- [src/gameplay/ghost.cpp](../src/gameplay/ghost.cpp) — per-ghost movement, mode lifecycle, render
- [src/gameplay/ghost_ai.cpp](../src/gameplay/ghost_ai.cpp) — direction picking, target tiles
- [src/gameplay/modes.cpp](../src/gameplay/modes.cpp) — global Scatter↔Chase wave timer
- [src/gameplay/collision.cpp](../src/gameplay/collision.cpp) — Pac↔ghost overlap
- [src/world/pathfind.h](../src/world/pathfind.h) — `distance_squared`

## Movement model

Ghosts share Pac's `(col, row, offset)` model. Each fixed-step tick the
ghost advances `offset` by `speed × dt`; when `offset` hits 1.0 the
ghost crosses into the neighbouring tile and **asks the AI** which
direction to head from there. That single function call is where all
the personality lives.

Tunnel wrap is identical to Pac's — column normalises automatically when
leaving `[0, kCols)`.

### Speed multipliers

| Mode | Multiplier | Effective speed (tiles/sec) |
| --- | --- | --- |
| Chase / Scatter / InHouse | × 1.00 | 5.5 |
| Frightened | × 0.50 | 2.75 — slower than Pac, so Pac can catch up |
| Eaten | × 1.80 | 9.9 — eyes race home |

## The five modes

| Mode | What it means | Target tile | Direction picking |
| --- | --- | --- | --- |
| `Chase` | Hunt Pac via the kind-specific target | personality below | greedy |
| `Scatter` | Patrol home corner | scatter corner below | greedy |
| `Frightened` | Blue, edible, slow | none | random |
| `Eaten` | Eyes-only, racing home | `(13, 11)` | greedy |
| `InHouse` | Sitting in the house, waiting | n/a (no movement) | n/a |

Modes change in well-defined places:

- **Wave change** — `WaveTimer` flips Scatter↔Chase on a fixed schedule;
  active ghosts swap mode and reverse direction once.
- **Power pellet eaten** — `Game::trigger_frightened()` flips every
  active ghost (not Eaten, not InHouse) into Frightened with a 6-second
  countdown, plus a queued reverse.
- **Frightened expires** — Ghost reverts to whatever the global wave
  mode is now.
- **Pac eats a frightened ghost** — That ghost flips to Eaten.
- **Eaten ghost reaches `(13, 11)`** — Reverts to current wave mode.
- **InHouse release timer fires** — Ghost teleports to `(13, 11)` and
  enters the current wave mode.

### Wave timer schedule

Implemented in [modes.cpp](../src/gameplay/modes.cpp):

| Phase | Duration | Mode |
| --- | --- | --- |
| 0 | 7 s | Scatter |
| 1 | 20 s | Chase |
| 2 | 7 s | Scatter |
| 3 | 20 s | Chase |
| 4 | 5 s | Scatter |
| 5 | 20 s | Chase |
| 6 | 5 s | Scatter |
| 7+ | ∞ | Chase forever |

`consume_just_changed()` returns true exactly once per mode flip so the
game loop knows when to queue the reversal.

## Personality target tiles (Chase mode)

The four expressions in [ghost_ai.cpp::chase_target](../src/gameplay/ghost_ai.cpp)
are the four classic Pac-Man personalities. Everything else about the
AI is identical between ghosts.

| Ghost | Target tile | Behaviour |
| --- | --- | --- |
| **Blinky** (red) | Pac's tile | Direct chaser. Head-on hunter. |
| **Pinky** (pink) | 4 tiles ahead of Pac | Ambusher. Cuts you off at corners. |
| **Inky** (cyan) | Pivot from Blinky doubled | Flanker. Approaches from Blinky's opposite side. |
| **Clyde** (orange) | Pac's tile if `dist² > 64`, else his scatter corner | Coward. Chases when far, runs when close. |

**Inky's math** is interesting. Take a "pivot" tile two tiles ahead of
Pac. Compute the vector from Blinky to that pivot. Double it. That's
Inky's target. Net effect: Inky tries to put Pac between himself and
Blinky.

## Scatter corners

Off-grid on purpose — the ghost never reaches its corner and instead
patrols nearby:

| Ghost | Scatter target |
| --- | --- |
| Blinky | `(25, -3)` — top-right |
| Pinky | `(2, -3)` — top-left |
| Inky | `(27, 31)` — bottom-right |
| Clyde | `(0, 31)` — bottom-left |

## Direction picking — the algorithm

For Chase / Scatter / Eaten:

1. Enumerate `Up, Left, Down, Right` in that order.
2. **Reject reverse** — the canonical "no-reverse" rule.
3. **Reject walls** (with tunnel wrap).
4. Score each candidate by squared Euclidean distance from the
   neighbour tile to the target tile.
5. Pick the smallest. **Strict `<` tie-break**, so the first candidate
   in the enumeration order wins ties: **Up > Left > Down > Right**.
6. Dead-end fallback: allow reverse if no forward option exists.

For Frightened:

1. Enumerate the same four directions.
2. Reject reverse + walls.
3. **Pick uniformly at random** from the remaining options.
4. Dead-end fallback as above.

The randomness is `std::rand() % n`. Game randomness doesn't need a
better RNG; if it ever does, swap to [src/core/rng](../src/core/rng.h).

## Mode-change reversal

When a ghost's `pending_reverse` flag is set (by the game loop after a
wave change or a power-pellet pickup), the next `ghost_update` performs
a **mid-tile reversal**: the ghost is reframed as already in the tile
he was heading toward, with `offset = 1 - offset`, and `dir =
opposite(dir)`. Continuous render position is unchanged — only the
direction of motion flips.

This is the same trick Pac uses for U-turns. Visually it produces the
canonical "all four ghosts pivot at once" cue that the wave just changed.

## Ghost-house exit (P6 simplification)

The arcade has each ghost slowly walk out through the house door over
a few seconds. P6 short-circuits this: each non-Blinky ghost holds a
**release timer** (`Pinky 1 s`, `Inky 5 s`, `Clyde 14 s` from start of
play). When it fires, the ghost **teleports** to `(13, 11)` (the
corridor directly above the door) and switches to the current wave
mode.

Blinky is hardcoded to spawn at `(13, 11)` from the start so the chase
engages immediately.

The proper "walk out / walk in" animation is a P11 polish task.

## Eat-ghost chain

While at least one ghost is in Frightened mode, Pac can eat them for
escalating bonuses:

| Ghost eaten this window | Points |
| --- | --- |
| 1st | 200 |
| 2nd | 400 |
| 3rd | 800 |
| 4th | 1600 |

Implemented in [game.cpp::update](../src/core/game.cpp) as `200 << chain_idx`.
The chain resets every time Pac eats a fresh power pellet.

When Pac touches a ghost, the result depends on that ghost's mode:

| Ghost mode at touch | Result |
| --- | --- |
| Chase / Scatter | **Pac dies** — life lost, death beat begins |
| Frightened | **Pac eats it** — chain bonus, ghost flips to Eaten |
| Eaten | No-op (eyes can't kill or be eaten) |
| InHouse | No-op (the ghost isn't on the board yet) |

## Pac↔ghost collision math

In [collision.cpp](../src/gameplay/collision.cpp). Continuous-position
test, **not** tile equality:

```cpp
hit ⇔ (pac_x − ghost_x)² + (pac_y − ghost_y)² < 14²
```

Why not `pac.col == ghost.col`? Two actors heading toward each other on
the same row can pass through the boundary between two tiles within the
same frame and never share a `(col, row)` pair, even though their
sprites visually overlap for several frames.

## Death + respawn

Same machinery as P5: when Pac touches a hostile ghost, `Score::lose_life()`
runs immediately, both actors freeze for `kDyingDuration = 0.9 s`, then
either both respawn at their starting tiles (`respawn_actors()`) or the
game flips into a frozen game-over.

`respawn_actors()` resets the wave timer and the eat chain too —
otherwise dying mid-wave 5 (Chase) would respawn Pac into a phase he
couldn't have caused.

## Tuning knobs

| Knob | Value | File |
| --- | --- | --- |
| Ghost base speed | `5.5` t/s | [ghost.h](../src/gameplay/ghost.h) `Ghost::speed_tiles_sec` |
| Frightened multiplier | `× 0.50` | [ghost.cpp](../src/gameplay/ghost.cpp) `speed_multiplier` |
| Eaten multiplier | `× 1.80` | same |
| Frightened duration | `6.0 s` | [game.cpp](../src/core/game.cpp) `kFrightenedDuration` |
| Wave durations | `7/20/7/20/5/20/5` | [modes.cpp](../src/gameplay/modes.cpp) `kPhaseDurations` |
| Pinky look-ahead | `4` tiles | [ghost_ai.cpp](../src/gameplay/ghost_ai.cpp) `chase_target` |
| Clyde flee threshold | `8²` tiles | same |
| House release (Pinky/Inky/Clyde) | `1 / 5 / 14 s` | [ghost.cpp](../src/gameplay/ghost.cpp) `ghost_init` |
| Touch threshold | `14 px` | [collision.cpp](../src/gameplay/collision.cpp) |
| Death beat | `0.9 s` | [game.cpp](../src/core/game.cpp) `kDyingDuration` |

P7 will scale Pac and ghost speeds per level for difficulty; for now
they're constants.
