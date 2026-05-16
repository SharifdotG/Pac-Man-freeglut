# Pac-Man (freeglut + C++17)

A polished Pac-Man clone built for the **CSE 426 Computer Graphics Lab** (Fall 2025) term project. Single-player, Windows-first, fixed-function OpenGL via freeglut. **All dependencies are bundled in-tree** — clone, build, play, no system setup.

> Status: ✅ **feature-complete** — all 11 phases shipped. See [docs/00-overview.md](docs/00-overview.md) for the live phase board, or jump straight to [docs/01-build-and-run.md](docs/01-build-and-run.md) to play.

## Highlights

- **28 × 31 tile maze**, classic Pac-Man behaviour: greedy ghost AI, scatter / chase / frightened modes, tunnel wrap, eat-ghost chain (200 → 400 → 800 → 1600).
- **Four canonical ghost personalities** — Blinky direct, Pinky 4-tile ambush, Inky Blinky-pivot flank, Clyde flee-when-near.
- **Three visually-distinct levels** with rising difficulty: ghost speed × 1.08^level, frightened time × 0.85^level. Fruit bonuses (Cherry / Strawberry / Orange / Apple / Melon) by level.
- **`READY!` beat** — every level start and every death respawn pauses for 1.5 s (skippable) so you're never thrown back into a hostile maze with no warning.
- **Live TIME readout** in the HUD (MM:SS, counts only "danger time") + final time on the win screen — matches the assignment's *"win in the minimum time"* objective.
- **Full UI flow** — Splash → Main menu → Help → Audio settings → Ready → Playing → Pause → Game-Over / Win, all navigable with arrow keys / WASD + Enter / Esc.
- **Persistent hi-score & volume** in `%APPDATA%/pacman-freeglut/savedata.txt`.
- **F11 letterboxed fullscreen toggle** — preserves the 672:824 aspect ratio at any window size.
- **15 marks of bonus content**: unique ghost abilities (Blinky fire-burst, Clyde clones), three power-up perks (Freeze / Speed / Invisibility), particle effects + screen shake, eat-streak combo system with floating popups, extra-life award at 10 000.
- **Procedurally synthesized chiptune audio** — 8 SFX + 1 looping BGM siren, generated in memory at startup via [miniaudio](https://github.com/mackron/miniaudio). No audio files shipped, no licensing risk, full chiptune timbre.
- **Zero third-party assets** — all sprites/sounds are drawn or synthesized procedurally. Only third-party code is freeglut and miniaudio (all permissive, all bundled).

## Quick start (Windows)

### Prerequisites

| Tool | Version | Where |
| --- | --- | --- |
| MinGW-w64 g++ | 13+ (tested with 15.2.0) | `C:/mingw64/` recommended; must be on `PATH` |
| CMake | 3.20+ (tested with 4.1.0) | on `PATH` |
| Git | any | for cloning |

You do **not** need freeglut, GLU, stb, or miniaudio installed system-wide — they're bundled in `thirdparty/`.

### Build & run

```bash
git clone https://github.com/SharifdotG/Pac-Man-freeglut.git
cd pacman-freeglut
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build -j
./build/bin/pacman.exe
```

The build copies `freeglut.dll` and the entire `assets/` folder next to `pacman.exe`, so you can also double-click the exe from File Explorer.

If you hit a build error, check [docs/12-build-troubleshooting.md](docs/12-build-troubleshooting.md).

## Download

If you just want to play without building, grab the latest release from the [Releases](https://github.com/SharifdotG/Pac-Man-freeglut/releases/download/v1.0.0/Pac-Man.freeglut.v1.0.0.zip) page. It's a zip of the `build/bin/` folder with everything needed to run. Unzip, then double-click `pacman.exe` to play. The hi-score and volume settings will persist across runs in `%APPDATA%/pacman-freeglut/savedata.txt`.

## Controls

| Key | Action |
| --- | --- |
| Arrow keys / WASD | Move Pac-Man (in Playing); navigate menus elsewhere |
| Left / Right | Adjust slider in Audio Settings |
| Enter | Confirm menu selection / skip splash |
| P | Pause / Resume |
| F11 | Toggle letterboxed fullscreen |
| Esc | Back one screen (Main Menu → quit) |
| Q | Hard-quit (any state) |

## Gameplay tips

- **Tap-ahead buffering**: tap a direction a few pixels before an intersection — Pac will turn there as soon as he arrives.
- **U-turn**: pressing the opposite of the current direction reverses Pac instantly mid-corridor.
- **Power pellets** turn ghosts blue; eat them in a chain for 200 / 400 / 800 / 1600 points.
- **Combo streak**: eat dots within 0.4 s of each other to ramp the multiplier from × 1.0 up to × 3.0.
- **Perks** (random spawn every 25–40 s): grab them for a few seconds of advantage.
- **Watch for Blinky's red halo** — he's about to fire a burst forward. Get out of the line.
- **Clyde occasionally clones himself** — translucent orange ghost that wanders for 4 s.

## Project layout

```text
src/                 game source (one folder per module)
  core/              app entry, state machine, clock, game orchestration
  render/            GL setup, primitives, text, particles, camera/shake
  audio/             miniaudio wrapper + in-memory WAV synthesis
  world/             tile/maze model + ASCII level loader + pathfind helpers
  gameplay/          pacman, ghosts, AI, modes, fruit, perks, score, combo, collision
  ui/                menu, help, pause, gameover/win, hud, settings
  input/             keyboard + edge-triggered press flags
  util/              direction enum, file/appdata helpers
assets/              level files (sprites / audio synthesized at runtime)
thirdparty/          freeglut, stb_image, miniaudio (no system install needed)
docs/                one numbered markdown per feature
.claude/skills/      agent how-to recipes for repeat workflows
```

See [docs/02-architecture.md](docs/02-architecture.md) for the full module breakdown.

## Documentation

| File | What's in it |
| --- | --- |
| [00-overview.md](docs/00-overview.md) | Phase status board + mark mapping |
| [01-build-and-run.md](docs/01-build-and-run.md) | Clone-to-play walkthrough + troubleshooting |
| [02-architecture.md](docs/02-architecture.md) | Module diagram, ownership tree, game loop |
| [03-tile-and-coordinates.md](docs/03-tile-and-coordinates.md) | Grid math, ortho projection, wall outline |
| [05-state-machine.md](docs/05-state-machine.md) | All states, transitions, key bindings |
| [06-ghost-ai.md](docs/06-ghost-ai.md) | Per-personality target tiles, modes, eat-chain |
| [07-input-and-buffering.md](docs/07-input-and-buffering.md) | Direction buffer, tap-ahead, U-turn |
| [08-audio.md](docs/08-audio.md) | Engine, in-memory WAV synthesis, SFX bank |
| [09-ui-and-menus.md](docs/09-ui-and-menus.md) | HUD layout + menu screens + letterbox |
| [10-creative-features.md](docs/10-creative-features.md) | The 4 bonus bundles (abilities, perks, particles, combo) |
| [11-level-format.md](docs/11-level-format.md) | ASCII grid legend + validation |
| [12-build-troubleshooting.md](docs/12-build-troubleshooting.md) | Common build/run issues |

## Mark mapping (45 / 45 target)

| Component | Marks | Where |
| --- | --- | --- |
| Base playable game | 20 | P1–P7 (window, maze, Pac, dots, ghosts, modes, levels) |
| Menu (start / resume / exit + navigation) | 5 | P8 — main menu, help, settings, pause, game-over |
| Pause / exit anytime | 5 | P8 — `P` toggles pause, `Esc` routes back, `Q` hard-quit |
| Bonus — unique ghost abilities | (15) | P10 — Blinky fire-burst, Clyde clone, plus canonical 4-personality AI |
| Bonus — power-up perks | (15) | P10 — Freeze, Speed, Invisibility |
| Bonus — particles + screen shake | (15) | P10 — sparkle, burst, dissolve, ring; death shake |
| Bonus — combo + popups | (15) | P10 — × 1.0–3.0 streak, floating score readouts |
| Bonus — help screen | (15) | P8 — controls + ghost legend with mini icons |
| Bonus — music + SFX | (15) | P9 — 8 procedurally-synthesized SFX + looping BGM siren + 3 volume sliders |
| Bonus — multi-level progression | (15) | P7 — 3 levels with rising difficulty + fruit bonuses + extra-life at 10 000 |

## License & credits

- **Code**: MIT (see [LICENSE](LICENSE)).
- **Sprites**: zero third-party sprite assets — Pac-Man, ghosts, dots, pellets, fruit and perk icons are all drawn procedurally with `glBegin(GL_TRIANGLE_FAN)` / `draw_filled_circle` / `draw_quad`. The pellet pulse, ghost-house door fade, and splash logo are all simple `sin()` envelopes over time.
- **Audio**: zero third-party sound files — all 8 SFX and the BGM siren are synthesized in memory at startup as 16-bit PCM WAV byte arrays via the small `Synth` helper in [src/audio/audio.cpp](src/audio/audio.cpp). See [docs/08-audio.md](docs/08-audio.md).
- **Third-party libraries** (bundled in [thirdparty/](thirdparty/)):
  - [freeglut](http://freeglut.sourceforge.net/) v3.8.0 — MIT/X11
  - [stb_image](https://github.com/nothings/stb) v2.30 — public domain (currently unused at runtime; kept for future asset support)
  - [miniaudio](https://github.com/mackron/miniaudio) v0.11.25 — public domain / MIT-0

> "PAC-MAN" is a registered trademark of Bandai Namco. This project is an **educational clone** for a coursework submission and is not affiliated with or endorsed by Namco. The maze layout is original-style; the ghost personalities follow the 1980 arcade behaviour as documented in public game-design literature.

## Course

Department of Computer Science and Engineering — **CSE 426 Computer Graphics Lab**, Fall 2025. Submitted as the term project (Pac-Man set, 30 base + 15 bonus marks).
