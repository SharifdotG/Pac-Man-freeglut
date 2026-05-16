# 04 — Asset Pipeline

The unusual headline first: **this project ships with zero sprite or
audio asset files**. Everything visible and audible is generated in
code at runtime. No PNG atlas, no WAV files, no font textures — just
`glBegin(GL_TRIANGLE_FAN)`, `draw_filled_circle`, `draw_quad`, and a
handful of `sin()` envelopes.

This is a deliberate design choice and not a TODO.

## Why no assets?

Three reasons:

1. **Licensing**: original Pac-Man art / sound is © Bandai Namco, and
   community fan-art derivatives sit in a grey area unsuitable for a
   public coursework repo.
2. **Build simplicity**: the repo is `git clone && cmake && build`,
   no asset pipeline / Pillow scripts / DCC tooling required.
3. **Tone**: a procedurally-drawn arcade game has the right pixel-shape
   feel for a 1980 Pac-Man clone. Crisper than a raster atlas at any
   physical resolution; no mipmap or filter concerns.

## What's drawn procedurally

| Visual | How it's drawn | File |
| --- | --- | --- |
| Pac-Man (yellow disc + animated mouth wedge) | `GL_TRIANGLE_FAN` skipping the mouth angle, half-angle modulated by `cos()` | [src/gameplay/pacman.cpp](../src/gameplay/pacman.cpp) |
| Ghosts (red/pink/cyan/orange rounded-top + flat bottom + eyes) | Top semicircle via `GL_TRIANGLE_FAN`, lower body via `draw_quad`, eyes as small filled circles, pupils offset by direction | [src/gameplay/ghost.cpp](../src/gameplay/ghost.cpp) |
| Frightened-mode ghost (deep blue with white-flash near expiry) | Same shape as above with palette swap based on `mode_timer` | [ghost.cpp](../src/gameplay/ghost.cpp) |
| Eaten ghost ("eyes only") | Body suppressed, eyes still drawn | [ghost.cpp](../src/gameplay/ghost.cpp) |
| Wall outline (iconic blue line maze) | For each wall tile, draw thin line segments along sides whose neighbour isn't a wall | [src/core/game.cpp](../src/core/game.cpp) `draw_wall_tile_edges` |
| Dots | Small filled `draw_quad` (4 × 4 px) | [game.cpp](../src/core/game.cpp) `render_maze` |
| Power pellets (animated pulse) | `draw_filled_circle` with radius modulated by `sin(t × 3.6)` | [game.cpp](../src/core/game.cpp) `render_maze` |
| Ghost-house door (fades open on release) | Pink horizontal bar with alpha = `1 − (timer / kDoorOpenDuration)` | [game.cpp](../src/core/game.cpp) `render_maze` |
| Fruit (Cherry / Strawberry / Orange / Apple / Melon) | Coloured `draw_filled_circle` + green `draw_quad` stem | [src/gameplay/fruit.cpp](../src/gameplay/fruit.cpp) |
| Perk icons (snowflake / chevron / crossed-eye) | Coloured disc + simple white glyph composed from quads | [src/gameplay/perks.cpp](../src/gameplay/perks.cpp) `perk_draw_icon` |
| Particles (sparkle / burst / ring / dissolve) | Each particle is a coloured `draw_quad` with alpha = `lifetime / max` | [src/render/particles.cpp](../src/render/particles.cpp) |
| HUD text (score, hi-score, level, "1UP", etc.) | `glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18)` | [src/render/text.cpp](../src/render/text.cpp) |
| Menu text + popups | Same `draw_text` API as HUD | various ui/*.cpp |
| Mini Pac icon (lives, splash) | `GL_TRIANGLE_FAN` shaped wedge | [src/ui/hud.cpp](../src/ui/hud.cpp), [game.cpp](../src/core/game.cpp) `render_splash` |
| Mini ghost icons (help legend) | Hand-drawn in `ui/help.cpp` to avoid a Ghost-struct round-trip | [src/ui/help.cpp](../src/ui/help.cpp) |
| Splash screen (fade-in / fade-out) | Triangular alpha envelope over `m_state_timer / kSplashDuration` | [game.cpp](../src/core/game.cpp) `render_splash` |

## What's synthesized procedurally (audio)

| Sound | Patch |
| --- | --- |
| Chomp | 60 ms 560 Hz square, fade |
| Power pellet | 4-note trill 540↑820↓540↑820 |
| Eat ghost | 300 ms sweep 220→880 Hz |
| Eat fruit | 2-note arpeggio 880, 1320 Hz |
| Death | 1.2 s descending sweep 880→110 Hz |
| Extra life | C5/E5/G5 major triad |
| Level start | C5/E5/G5/C6 ascending |
| Menu click | 30 ms 440 Hz square blip |
| BGM | 4-second wobbling square loop, 140 ± 40 Hz at 2 Hz wobble |

Implementation: a small `Synth` class in
[src/audio/audio.cpp](../src/audio/audio.cpp) supports
`add_sine`, `add_square`, `add_sweep`, `add_silence`, and
`add_wobble_square`. Each sound is built up at startup, wrapped in a
minimal RIFF/WAVE/PCM header (`Synth::to_wav()`), and handed to
miniaudio's in-memory decoder. See [08-audio.md](08-audio.md).

## On-disk asset files

Only one kind of asset file ships: **maze level files** under
[assets/levels/](../assets/levels/). They're 28 × 31 character ASCII
grids parsed at level-load time. See
[11-level-format.md](11-level-format.md) for the legend.

There's also a `tools/` directory in the repo for one-off scripts, but
nothing in it ships with the game — it's a graveyard for any
authoring scripts a contributor might use.

## Adding a real sprite atlas later

If you want to swap procedural drawing for a real PNG atlas:

1. Add a `tools/gen_atlas.py` (Python + Pillow) that writes
   `assets/sprites/atlas.png`.
2. Switch `render/texture.cpp` from a stub to a real loader: define
   `STB_IMAGE_IMPLEMENTATION` above the `#include <stb_image.h>` and
   provide a `Texture` RAII handle owning a GL texture id.
3. Add an `assets/sprites/atlas.json` (or build one inline in code) with
   named UV rects per sprite.
4. Replace the `draw_filled_circle` / `draw_quad` calls in pacman.cpp,
   ghost.cpp, fruit.cpp, etc., with `render::draw_sprite("name", x, y)`.
5. Update CMake's POST_BUILD copy step to include `assets/sprites/`.

The infrastructure for this exists: [src/render/atlas.cpp](../src/render/atlas.cpp)
and [src/render/sprite.cpp](../src/render/sprite.cpp) are stubbed, and
the [add-sprite skill](../.claude/skills/add-sprite.md) walks through
the workflow.

But: the game *looks complete* with procedural art. This swap is only
worth doing if you want a different visual style.
