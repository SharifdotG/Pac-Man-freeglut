# 09 — UI, HUD & Menus

This doc covers the visible chrome around the play area: HUD strip, main
menu, help screen, pause overlay, and game-over / win screens.

Authoritative code: [src/ui/](../src/ui/) — one file per screen — plus
[src/render/text.cpp](../src/render/text.cpp) for the actual character
drawing.

## Common rendering primitives

All UI text uses freeglut's built-in `GLUT_BITMAP_HELVETICA_18` via
[`render::draw_text`](../src/render/text.h). Centred placement uses
`render::draw_text_centered`, which measures the string with
`glutBitmapWidth` and offsets accordingly. Right-side-up under our y-down
ortho because `glBitmap` is a raster op fixed in framebuffer space — see
the explanation in `text.cpp`.

Colour palette (kept consistent across screens):

| Use | Colour |
| --- | --- |
| Title text | yellow `{1.00, 0.92, 0.16}` (Pac's body colour) |
| Selected menu item | bright yellow `{1.00, 0.95, 0.30}` |
| Inactive menu item | soft white `{0.85, 0.85, 0.95}` |
| Footer / hint | dim purple `{0.55, 0.55, 0.70}` |
| Win banner | green `{0.30, 1.00, 0.40}` |
| Game-over banner | red `{1.00, 0.30, 0.30}` |
| Score callout | yellow `{1.00, 0.95, 0.30}` |
| Translucent overlay | black at 65% alpha |

## HUD (P4) — bottom 80 px strip

Lives in `world_y in [744, 824]`. Implementation:
[src/ui/hud.cpp](../src/ui/hud.cpp). Reads from
[`Score`](../src/gameplay/score.h).

```text
+--------------------------------------------------------+
|  1UP            HIGH SCORE                  LEVEL 3    |
|  12340             56789                                |
|  ☻ ☻         TIME  03:42                       [perk]  |
+--------------------------------------------------------+
```

- Left block (x ≈ 24): "1UP" + score
- Centre: "HIGH SCORE" + hi-score (top), "TIME MM:SS" (bottom — P12)
- Right (right-aligned): "LEVEL N" (top), active-perk badge + timer (bottom)
- Lives row: yellow Pac-Man icons under the score, one per remaining life

The **TIME readout** (P12) counts only while the game is in the
`Playing` state — paused / dying / ready / level-complete time is
excluded so the timer reflects actual time-at-risk, matching the
assignment's *"win in the minimum time"* objective.

A 1.5 px dim-blue separator line at y ≈ 746 marks the play-area / HUD
boundary.

## Main menu (P8) — entry state

Implementation: [src/ui/menu.cpp](../src/ui/menu.cpp).

```text
                    PAC-MAN
                  freeglut + C++17

                  >  START
                     HELP
                     EXIT

              HIGH SCORE: 12340

       Use Arrow keys / WASD + Enter to navigate
        CSE 426 Computer Graphics Lab — Spring 2025
```

`render_main_menu(selected, hi_score)` takes the cursor index (0..2) and
the persistent hi-score. The hi-score line is suppressed on the very
first run when it's still 0.

## Help screen (P8)

Implementation: [src/ui/help.cpp](../src/ui/help.cpp). Two columns:
controls on the left, action description on the right. Below that, a
ghost legend with miniature procedural ghost icons drawn directly in
help.cpp (so the help screen doesn't depend on instantiating actual
Ghost structs).

Press **Esc** or **Enter** to return.

## Pause overlay (P8)

Implementation: [src/ui/pause.cpp](../src/ui/pause.cpp).

Render order:

1. Game::render renders the (frozen) play scene + HUD as normal.
2. `render_pause_overlay(selected)` draws a 65%-alpha black quad over the
   play area only (HUD remains fully bright).
3. The "PAUSED" header + RESUME / MAIN MENU options layer on top.

Pressing **P**, **Esc**, or selecting **RESUME** returns to Playing —
**without** resetting the cursor (so re-pausing remembers the user's
last selection, a small UX nicety).

## Game-Over and Win screens (P8)

Implementation: [src/ui/gameover.cpp](../src/ui/gameover.cpp). Both
share the same layout — only the title text and colour differ.

```text
              GAME OVER  /  YOU WIN!

           FINAL SCORE: 12340

              >  RETRY  /  PLAY AGAIN
                 MAIN MENU

           Arrow keys + Enter to choose
```

- **Retry / Play Again** → `start_new_game()` → resets score, loads
  level 1, transitions to Playing.
- **Main Menu** → returns to the title screen with score preserved as
  hi-score (if appropriate).

The play scene underneath is frozen — players see the maze state at the
moment they lost / won, with the dim overlay over the play area only so
the HUD stays readable.

## Letterboxed fullscreen (P8)

F11 toggles fullscreen via `glutFullScreen()` /
`glutReshapeWindow(672, 824)`. The reshape callback in
[render/gl_init.cpp](../src/render/gl_init.cpp) computes the largest
sub-rect that preserves the 672:824 aspect ratio. Bars are filled with
black via a full-framebuffer clear before the letterbox viewport is
re-applied for drawing.

UI screens are designed at the logical 672 × 824 resolution and scale
proportionally to whatever physical window size is in use. No element
relies on the actual pixel dimensions.

## P12 polish (delivered)

- **Pulsing selected items** — every menu (Main, Pause, GameOver, Win,
  Settings) modulates the selected row's colour with `0.85 + 0.15 ×
  sin(t × 6)` for a heartbeat-like attention cue.
- **Title accent lines** — short yellow horizontal bars under the
  PAC-MAN / PAUSED / HOW TO PLAY / AUDIO SETTINGS / GAME OVER / YOU
  WIN! titles. Subtle visual anchor without adding any text.
- **Decorative dots** — two small Pac-style dots flank the main-menu
  title.
- **Better vertical rhythm** — menu line spacing bumped from 36–40 px
  to 40–48 px so the items don't crowd the title.
- **`READY!` beat** — after every level load and every death respawn,
  the game enters `GameState::Ready` for 1.5 s with a centred yellow
  pulsing banner. Skippable on any keypress. Players get a moment to
  orient themselves before the ghosts can hit them.

## What's NOT in this pass (potential future polish)

- Fade transitions between states (currently snap)
- Pixel-art bitmap font replacing Helvetica
- A spinning Pac-Man logo on the main menu
