# CLAUDE.md — guidance for Claude / Copilot when working on this repo

This file is auto-loaded into Claude Code conversations in this directory.
Read it before suggesting changes; keep it up to date when conventions evolve.

## What this is

A Pac-Man clone built in **C++17 + freeglut** for the **CSE 426 Computer
Graphics Lab** term project. Single-player, Windows-only, runs on
fixed-function OpenGL 1.x with textured quads. Targets 20 (base) + 5 (menu)

+ 5 (pause/exit) + 15 (creativity) = 45 marks.

The full plan is at `C:\Users\SharifdotG\.claude\plans\hey-claude-the-given-golden-lobster.md`
(referenced from MEMORY.md).

## Repo layout

```plaintext
src/
  main.cpp          GLUT init + main loop entry point
  core/             timer, top-level Game state machine, app glue, rng
  render/           gl_init, texture, atlas, sprite, primitives, text, particles, camera (shake)
  audio/            miniaudio wrapper
  world/            tile, maze, level_loader, pathfind
  gameplay/         pacman, ghost, ghost_ai, modes, fruit, perks, score, combo, collision
  ui/               menu, help, pause, gameover, hud, transitions
  input/            input
  util/             vec2.h, rect.h, color.h, file, log
assets/             sprites/, audio/, fonts/, levels/  — copied next to exe at build time
thirdparty/         freeglut binaries + headers, stb_image.h, miniaudio.h
docs/               one numbered .md per feature; read in order
.claude/skills/     reusable how-tos for repeat tasks (verify-phase, add-level, add-sprite, add-sound, add-bonus-feature)
tools/              one-off scripts (atlas packer, etc.) — NOT shipped to graders
```

## Build & run

See [docs/01-build-and-run.md](docs/01-build-and-run.md). TL;DR:

```bash
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build -j
./build/bin/pacman.exe
```

There are **no unit tests** — verification is manual per the per-phase verify
steps in the plan. The user grades by demo, not by test suite.

## Conventions

+ **C++17.** No exceptions across module boundaries; use `std::optional` /
  return-codes for fallible ops. Plain structs + free functions over heavy OOP
  unless the domain demands it (only `Ghost` has subclasses).
+ **No raw `new`/`delete`.** Use stack values, `std::vector`, `std::unique_ptr`.
+ **Header guards:** `#pragma once`.
+ **Includes:** project headers in `""`, system / third-party in `<>`. Group
  std → third-party → project, with one blank line between groups.
+ **Comments:** explain **why**, never **what**. If a line is non-obvious,
  prefer renaming the symbol over adding a comment.
+ **Naming:** `snake_case` for free functions and variables, `PascalCase` for
  types, `kPascalCase` for constants, `m_` prefix on private members.
+ **No globals** except `extern` accessors for the audio engine and the input
  state (both are owned by `Game`).
+ **No abstract base classes** for things with one implementation. YAGNI hard.
+ **Each .cpp ≤ ~300 lines.** Split by concern, not by line count target.
+ **Strict warnings on:** `-Wall -Wextra -Wpedantic -Wshadow -Wconversion`.
  Treat warnings as work to do.

## Tile / coordinate system

+ Maze grid: **28 columns × 31 rows**, tile size **24 px** → play area 672×744.
+ Window: **672 × 824** (HUD lives in the bottom 80 px, screen-space pixel coords).
+ Origin (0,0) is **top-left** (set via `gluOrtho2D(0, w, h, 0)`).
+ Tile (col, row) → pixel (col*24, row*24).

## Asset rules

+ **Sprites** are PNGs in `assets/sprites/`. Loaded at startup via `stb_image`.
  The renderer uses a single sprite atlas (`atlas.png` + `atlas.json` with
  named UV rects), not per-frame files.
+ **Audio** files live in `assets/audio/`. WAV for SFX, OGG for the loop BGM.
+ **Levels** are ASCII grids in `assets/levels/levelNN.txt`. See
  [docs/11-level-format.md](docs/11-level-format.md) for the character legend.
+ The build copies `assets/` next to the exe via `add_custom_command(POST_BUILD)`,
  so the game finds them with `assets/...` relative paths.

## State machine

Top-level `Game` runs one of these states each frame:
`MENU → HELP → PLAYING → PAUSED → LEVEL_COMPLETE → GAME_OVER → WIN_SCREEN → MENU…`

Transitions happen in [src/core/game.cpp](src/core/game.cpp). Don't sprinkle
state changes throughout ghost / pacman code — fire events upward and let
`Game` decide.

## Things to NOT do

+ Don't add a CMake `find_package(GLUT)` — we deliberately ship our own copy in
  `thirdparty/freeglut/`.
+ Don't load files with absolute paths. Always relative to the working directory
  (which is `build/bin/` when running from `cmake --build` output).
+ Don't introduce a new third-party dependency without bundling it in
  `thirdparty/`. The whole point is "git clone && build, no system setup."
+ Don't add unit-test scaffolding. The user verifies by demoing.
+ Don't `git commit` / `git push` / open PRs. The user does all git work.

## When you finish a feature

1. Add or update its `docs/NN-*.md` with: what it does, where it lives, how to
   tweak it, any gotchas.
2. If a workflow becomes repeatable (adding a level, swapping a sprite),
   write or update a `.claude/skills/<name>.md`.
3. Update this file if conventions changed.
4. Stop. Do not commit. Tell the user the phase is done.
