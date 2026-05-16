# 00 — Overview

## What this is

A Pac-Man clone for the **CSE 426 Computer Graphics Lab** (Fall 2025) term project.
C++17 + freeglut, fixed-function OpenGL 1.x, single-player, Windows-first, all dependencies bundled.

**Status:** ✅ feature-complete — all 12 phases shipped (P0–P11 build, P12 polish + cleanup). `cmake -G "MinGW Makefiles" -S . -B build && cmake --build build && build/bin/pacman.exe` boots into the splash, lands on the menu, and plays through 3 visually-distinct levels with full audio + a "READY!" beat between deaths and level transitions + a live TIME readout in the HUD + all the P10 bonus features.

## Mark mapping (45 / 45 target)

| Component | Marks | Status |
| --- | --- | --- |
| Base playable game | 20 | ✅ P1–P7 |
| Menu + navigation | 5 | ✅ P8 |
| Pause / exit anytime | 5 | ✅ P8 |
| Bonus — unique ghost abilities (Blinky fire / Clyde clone) | _of 15_ | ✅ P10 |
| Bonus — power-up perks (Freeze / Speed / Invisibility) | _of 15_ | ✅ P10 |
| Bonus — particles + screen shake | _of 15_ | ✅ P10 |
| Bonus — combo system + popups | _of 15_ | ✅ P10 |
| Bonus — help screen | _of 15_ | ✅ P8 |
| Bonus — music + SFX + volume sliders | _of 15_ | ✅ P9 |
| Bonus — multi-level progression + difficulty + fruit | _of 15_ | ✅ P7 |
| Polish — splash, animated pellets, hi-score persist, door anim | _of 15_ | ✅ P11 |
| **Total target** | **45** | **delivered** |

## Phase status board

| Phase | Title | Status |
| --- | --- | --- |
| P0 | Repo scaffold | ✅ complete |
| P1 | Window + game loop | ✅ complete |
| P2 | Maze rendering | ✅ complete |
| P3 | Pac-Man movement | ✅ complete |
| P4 | Dots, score, lives, HUD | ✅ complete |
| P5 | Blinky + collision | ✅ complete |
| P6 | Four ghosts + modes | ✅ complete |
| P7 | Win / lose / levels | ✅ complete |
| P8 | UI screens + state machine | ✅ complete |
| P9 | Audio | ✅ complete |
| P10 | Creative features | ✅ complete |
| P11 | Polish | ✅ complete |
| P12 | Final cleanup + QoL polish | ✅ complete |

## Architecture (high-level)

- **Window:** 672 × 824 logical, top-left origin via `gluOrtho2D(0, 672, 824, 0)`. F11 toggles letterboxed fullscreen.
- **Play area:** 28 cols × 31 rows × 24 px = 672 × 744. HUD lives in the bottom 80 px.
- **State machine** in [src/core/game.cpp](../src/core/game.cpp):
  `Splash → Menu → Help / Settings / Playing → Paused → Dying → LevelComplete → GameOver / WinScreen`.
- **Rendering:** immediate-mode `glBegin/glEnd`, **all sprites drawn procedurally** (no texture/atlas pipeline at runtime). Particles, screen shake, animated pellets and door fade are all simple `sin()` envelopes.
- **Audio:** miniaudio, **all SFX + BGM synthesized in memory at startup** (no audio asset files).
- **Persistent user data:** `%APPDATA%/pacman-freeglut/savedata.txt` — hi-score + master/SFX/BGM volumes.

For a deeper dive, see [02-architecture.md](02-architecture.md).

## Conventions

See [CLAUDE.md](../CLAUDE.md) at the repo root for code style, naming, and "things to NOT do."

## Where to start

1. Read [01-build-and-run.md](01-build-and-run.md) — clone-to-play walkthrough.
2. Read [CLAUDE.md](../CLAUDE.md) — code style.
3. Read the implementation plan at `C:\Users\SharifdotG\.claude\plans\hey-claude-the-given-golden-lobster.md` for the full phase breakdown.
