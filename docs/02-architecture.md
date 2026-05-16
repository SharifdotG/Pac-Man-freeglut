# 02 — Architecture

This file is the **module map** of the project. Every other doc drills into a
specific subsystem; this is the index. Updated when a phase introduces or
restructures a module.

## Top-level entry point

```plaintext
main.cpp
  └─ core::run_app(argc, argv)        // src/core/app.cpp
       ├─ glutInit + window create
       ├─ render::init_gl              // src/render/gl_init.cpp
       ├─ register GLUT callbacks
       ├─ glutMainLoop
       │     └─ on_idle (every frame)
       │           ├─ core::Clock::tick()    fixed-step accumulator
       │           ├─ <run N updates>        (P3+)
       │           └─ glutPostRedisplay → on_display
       └─ "clean shutdown" log
```

## Module ownership

| Module | Path | Phase added | Owns |
| --- | --- | --- | --- |
| core | [src/core/](../src/core/) | P1 | App entry, fixed-step clock; will own Game state machine in P3+ |
| render | [src/render/](../src/render/) | P1 | GL state init, ortho projection; sprites + atlas + text + particles + camera land in later phases |
| input | [src/input/](../src/input/) | P1 | Keyboard state, GLUT key callbacks |
| audio | [src/audio/](../src/audio/) | P9 | miniaudio engine wrapper |
| world | [src/world/](../src/world/) | P2 | Tile constants, maze grid, ASCII level loader, pathfinding helpers |
| gameplay | [src/gameplay/](../src/gameplay/) | P3+ | Pac-Man, ghosts, AI, perks, score, combo, fruit |
| ui | [src/ui/](../src/ui/) | P4+ | HUD, menus, transitions |
| util | [src/util/](../src/util/) | as-needed | Vec2/Rect/Color POD math, file/log helpers |

## Phase 1: the game loop

The current loop is bare-bones — no game content, just timing. Two functions
matter:

### [core::Clock](../src/core/clock.h)

Drives a **fixed-timestep accumulator** at 60 Hz. Each frame:

1. `tick()` measures wall-clock dt, adds it to the accumulator, returns the
   number of fixed-dt steps to run.
2. `dt > 0.25 s` is clamped to prevent a "spiral of death" if the window
   stalls (e.g. when dragged or the debugger pauses).
3. `fps_ready()` rolls a 1-second window; when full, `last_fps()` and
   `last_ups()` give the average frame and update rates.

Why fixed step? Game logic (ghost AI, collision, animation) needs to run
deterministically regardless of vsync wobble. Render rate floats; sim rate
doesn't.

### [core::run_app](../src/core/app.cpp)

Single function that owns the GLUT lifecycle. Holds the only `Clock`
instance and the FPS-printing logic. The display callback currently does
nothing but `glClear + swap`; later phases will drive scene rendering from
`Game::render(state)`.

ESC, Q, and the window-X all set `input::state().quit_requested = true`;
the idle callback checks that gate once per frame and calls
`glutLeaveMainLoop()`.

## What's NOT in P1 (intentionally)

- No game state machine yet — that's P3 / P8.
- No texture loading — that's P2 (`render::texture` flips on
  `STB_IMAGE_IMPLEMENTATION`).
- No audio — P9 (`audio::audio` flips on `MINIAUDIO_IMPLEMENTATION`).
- No assets are read at runtime yet, so `assets/` next to the exe is empty
  but harmless.

## Verifying the loop

Run `pacman.exe` from a terminal. You should see one log line per second:

```powershell
[pacman] window opened (672x824). target update rate: 60 Hz.
[pacman] fps=… ups=…
[pacman] fps=… ups=…
…
[pacman] clean shutdown.
```

`fps` is the render rate (capped by your display vsync — typically 60 or 144).
`ups` should hover at 60 regardless. If `ups` drifts measurably from 60, the
accumulator math is wrong; investigate before moving on to P2.
