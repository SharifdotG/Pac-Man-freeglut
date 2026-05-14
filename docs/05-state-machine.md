# 05 — Game State Machine

The whole game is a single state machine. Every frame, exactly one state is
active; that state owns the update and render dispatch.

Authoritative code: [src/core/state.h](../src/core/state.h) (enum) and
[src/core/game.cpp](../src/core/game.cpp) (transitions + per-state
update/render).

## States

| State | Active simulation? | Renders | Entry from | Exits to |
| --- | --- | --- | --- | --- |
| `Splash` | no | logo fade-in/out | boot | `Menu` (after 1.5 s or any keypress) |
| `Menu` | no | menu screen only | Splash, every back-out path | `Help`, `Settings`, `Ready` (Start), quit |
| `Help` | no | help screen only | Menu | Menu |
| `Settings` | no | three volume sliders | Menu | Menu |
| `Ready` | no (frozen) | play scene + yellow READY! banner | level load OR death respawn | `Playing` (after 1.5 s or any keypress) |
| `Playing` | YES | full play scene + HUD | `Ready` | `Paused`, `Dying`, `LevelComplete` |
| `Paused` | no (frozen) | play scene + dim overlay + pause menu | Playing | Playing, Menu |
| `Dying` | partially | play scene with Pac freezing & vanishing | Playing (Pac↔ghost touch) | `Ready` (respawn) or `GameOver` |
| `LevelComplete` | partially | play scene with banner | Playing (last dot eaten) | `Ready` (next level) or `WinScreen` |
| `GameOver` | no (frozen) | play scene + game-over screen | Dying (lives = 0) | Menu, restart |
| `WinScreen` | no (frozen) | play scene + win screen | LevelComplete (final level) | Menu, restart |

## Transition diagram

```text
                      ┌──────────────────┐
        boot ─────────► Menu            │
                      └──┬───────────────┘
                         │ Start                 ┌──────────┐
                         ▼                       │   Help   │◄──┐
                      ┌──────────────────┐       └──────────┘   │
                ┌─────► Playing          │            ▲         │
                │     └──┬─┬─┬───────────┘            │         │
                │        │ │ │                  Help item  Esc/Enter
                │        │ │ │                        │         │
                │        │ │ └─► LevelComplete        │         │
                │        │ │       │                  │         │
                │        │ │       ├─► Playing (next) │         │
                │        │ │       └─► WinScreen ─────┼─────────┤
                │        │ │                          │         │
                │        │ └─► Dying                  │         │
                │        │       │                    │         │
                │        │       ├─► Playing (respawn)│         │
                │        │       └─► GameOver ────────┼─────────┤
                │        │                            │         │
                │        └─► Paused ──┬─────────► Playing       │
                │                     └─────────► Menu ─────────┘
                │
                └─── Retry / Play Again from GameOver / WinScreen
```

## Inputs that drive transitions

| Key | Effect by state |
| --- | --- |
| **Arrow keys / WASD** | Pac direction (Playing) AND menu navigation (any state with menu) |
| **Enter** | Confirm current menu selection |
| **Esc** | "Back one screen". Menu → quit, Help/Game-Over → Menu, Playing → Paused, Paused → Playing, etc. |
| **P** | Toggle Playing ↔ Paused |
| **Q** | Hard quit (any state) |
| **F11** | Toggle fullscreen (any state) |
| **Window-X** | Hard quit |

The `press_*` flags in [src/input/input.h](../src/input/input.h) are
**edge-triggered** — set on the frame the key is first pressed and cleared
by `clear_press_flags()` at the end of `Game::update()`. This makes them
work as one-shot events for menu navigation (a single press = one cursor
move), while `wanted` (Pac's direction buffer) stays sticky across frames.

## Menu selection

There's a single `m_menu_selection` integer in `Game`, reset to 0 by
`enter_state()`. Each menu-bearing state interprets it against its own
item count:

| State | 0 | 1 | 2 |
| --- | --- | --- | --- |
| `Menu` | START | HELP | EXIT |
| `Paused` | RESUME | MAIN MENU | — |
| `GameOver` | RETRY | MAIN MENU | — |
| `WinScreen` | PLAY AGAIN | MAIN MENU | — |

Up/Down wraps around modulo the count.

## What's frozen and what isn't

**Frozen** (no game-loop ticks): `Menu`, `Help`, `Paused`, `GameOver`,
`WinScreen`. The wave timer, ghost AI, fruit lifetime, and Pac all stop.

**Partially frozen** (state-timer ticks, gameplay paused): `Dying`,
`LevelComplete`. The actor updates don't run, but the death/celebration
beat counts up.

**Live**: `Playing`. Everything ticks.

## Why `Paused` doesn't reset `m_menu_selection`

Most states use `enter_state()` which resets the cursor to 0. Pause is
slightly different: while paused, the cursor sits on whatever the user
last selected. Resume returns to play **without** zeroing the cursor.
This means re-pausing remembers the previous selection — small UX nicety.

The implementation does this by manually setting `m_state` and
`m_state_timer` instead of calling `enter_state()`.

## Fullscreen toggle (F11)

Implemented in [core/app.cpp](../src/core/app.cpp) (`toggle_fullscreen`)
plus [render/gl_init.cpp](../src/render/gl_init.cpp) (letterbox math).

- `glutFullScreen()` to enter fullscreen; `glutReshapeWindow(672, 824)`
  to return windowed.
- The `on_reshape` callback computes a centred sub-rect that preserves
  the canonical 672:824 aspect ratio. The result is **letterboxed** when
  the window is wider than 672:824 and **pillarboxed** when taller.
- `clear_and_apply_viewport()` runs at the start of every display frame:
  clears the *full* framebuffer to black (so the bars are clean), then
  restores the aspect-preserving viewport for scene drawing.

The logical resolution stays at 672 × 824 in world coordinates regardless
of the actual window pixel size — UI layouts and menus can hard-code
positions and they'll always render proportionally.
