# Live-Modification Cheat Sheet — Lab Final Exam Prep

> **Use this when the teacher says: *"Now change X to Y and rebuild."***
>
> Each row gives you the **exact file**, the **exact line** (and what to type), and the **rebuild command** so you don't fumble during the demo.
>
> **Universal rebuild command after any change:**
>
> ```bash
> cmake --build build -j
> ./build/bin/pacman.exe
> ```
>
> If you haven't built once yet:
>
> ```bash
> cmake -G "MinGW Makefiles" -S . -B build
> cmake --build build -j
> ./build/bin/pacman.exe
> ```

---

## Table of contents

- [Live-Modification Cheat Sheet — Lab Final Exam Prep](#live-modification-cheat-sheet--lab-final-exam-prep)
  - [Table of contents](#table-of-contents)
- [1. Lives count (3 → 4 → N)](#1-lives-count-3--4--n)
- [2. Point values (dot, pellet, ghost, fruit)](#2-point-values-dot-pellet-ghost-fruit)
    - [2.1 Dot value (default 10)](#21-dot-value-default-10)
    - [2.2 Power-pellet value (default 50)](#22-power-pellet-value-default-50)
    - [2.3 Ghost eat-chain values (default 200, 400, 800, 1600)](#23-ghost-eat-chain-values-default-200-400-800-1600)
    - [2.4 Fruit values (default 100, 300, 500, 700, 1000)](#24-fruit-values-default-100-300-500-700-1000)
- [3. Extra-life threshold](#3-extra-life-threshold)
- [4. Pac-Man speed](#4-pac-man-speed)
- [5. Ghost speed](#5-ghost-speed)
- [6. Frightened (blue-ghost) duration](#6-frightened-blue-ghost-duration)
- [7. Per-level difficulty scaling](#7-per-level-difficulty-scaling)
- [8. Number of levels](#8-number-of-levels)
- [9. Maze size or tile size](#9-maze-size-or-tile-size)
- [10. Window / aspect ratio](#10-window--aspect-ratio)
- [11. Ghost-house release timings](#11-ghost-house-release-timings)
- [12. Ghost colours](#12-ghost-colours)
- [13. Pac-Man colour or radius](#13-pac-man-colour-or-radius)
- [14. Pac-Man mouth animation speed](#14-pac-man-mouth-animation-speed)
- [15. Wave (Scatter/Chase) schedule](#15-wave-scatterchase-schedule)
- [16. Ghost AI behaviour (personalities)](#16-ghost-ai-behaviour-personalities)
- [17. Collision distance](#17-collision-distance)
- [18. Power-pellet visual pulse rate](#18-power-pellet-visual-pulse-rate)
- [19. Splash / READY! / Dying / LevelComplete durations](#19-splash--ready--dying--levelcomplete-durations)
- [20. Fruit spawn thresholds + spawn tile](#20-fruit-spawn-thresholds--spawn-tile)
    - [Thresholds (dots eaten before fruit appears)](#thresholds-dots-eaten-before-fruit-appears)
    - [Spawn tile coordinates](#spawn-tile-coordinates)
    - [Fruit lifetime on screen](#fruit-lifetime-on-screen)
- [21. Perk tuning (durations, spawn timing, speed multiplier)](#21-perk-tuning-durations-spawn-timing-speed-multiplier)
- [22. Combo / chain multiplier](#22-combo--chain-multiplier)
- [23. Blinky fire-burst tuning](#23-blinky-fire-burst-tuning)
- [24. Clyde clone tuning](#24-clyde-clone-tuning)
- [25. Keyboard bindings](#25-keyboard-bindings)
- [26. Game / window title](#26-game--window-title)
- [27. Frame rate / update tick](#27-frame-rate--update-tick)
    - [Update rate (default 60 Hz)](#update-rate-default-60-hz)
    - [GLUT timer interval (default 16 ms ≈ 62.5 Hz)](#glut-timer-interval-default-16-ms--625-hz)
- [28. Camera shake intensity / duration](#28-camera-shake-intensity--duration)
- [29. Audio volumes (defaults)](#29-audio-volumes-defaults)
- [30. Maze / wall / dot colours](#30-maze--wall--dot-colours)
- [31. Replace / edit a level (the ASCII files)](#31-replace--edit-a-level-the-ascii-files)
- [32. Add a brand-new level](#32-add-a-brand-new-level)
- [33. Common "what if it doesn't build" fixes](#33-common-what-if-it-doesnt-build-fixes)
    - ["Cannot find freeglut.dll"](#cannot-find-freeglutdll)
    - ["level parse error: expected 31 rows, got 30"](#level-parse-error-expected-31-rows-got-30)
    - ["level parse error: missing one of P/B/I/N/C spawn markers"](#level-parse-error-missing-one-of-pbinc-spawn-markers)
    - ["level row N col M: bad char 'X'"](#level-row-n-col-m-bad-char-x)
    - [After changing tile size, Pac falls into walls](#after-changing-tile-size-pac-falls-into-walls)
    - [The game compiles but ghosts don't move](#the-game-compiles-but-ghosts-dont-move)
    - [Pac-Man teleports / glitches](#pac-man-teleports--glitches)
    - [Build error after deleting a `.cpp` file](#build-error-after-deleting-a-cpp-file)
    - [Rebuild "doesn't seem to do anything"](#rebuild-doesnt-seem-to-do-anything)
  - [Top 10 most-likely exam modifications, by probability](#top-10-most-likely-exam-modifications-by-probability)
  - [The 60-second emergency drill](#the-60-second-emergency-drill)

---

# 1. Lives count (3 → 4 → N)

**Most likely exam request.** Change the starting-lives constant.

**File:** [src/gameplay/score.h](../src/gameplay/score.h) — line 10.

**Find:**

```cpp
static constexpr int kStartingLives = 3;
```

**Replace `3` with whatever the teacher asks** (4, 5, 1, 10…).

That single edit is enough — `Score::reset_for_new_game()` reads from it.

**Verification after rebuild:** Start a new game, count Pac-Man icons in the bottom-left HUD strip.

---

# 2. Point values (dot, pellet, ghost, fruit)

### 2.1 Dot value (default 10)

**File:** [src/gameplay/score.h](../src/gameplay/score.h) — line 11.

```cpp
static constexpr int kPointsPerDot = 10;
```

> **⚠ Beware:** The `Maze::eat_at()` switch hard-codes `return 10;` and `return 50;` at [src/world/maze.cpp:14,18](../src/world/maze.cpp#L14). The constant in score.h is the one consumed by *combo math* — if the teacher wants the actual dot pickup value changed, edit BOTH places:
>
> - [src/gameplay/score.h:11](../src/gameplay/score.h#L11) → `kPointsPerDot`
> - [src/world/maze.cpp:14](../src/world/maze.cpp#L14) → `return 10;` (change the literal)

### 2.2 Power-pellet value (default 50)

Same files. Edit BOTH:

- [src/gameplay/score.h:12](../src/gameplay/score.h#L12) — `kPointsPerPellet = 50;`
- [src/world/maze.cpp:18](../src/world/maze.cpp#L18) — `return 50;`

### 2.3 Ghost eat-chain values (default 200, 400, 800, 1600)

**File:** [src/core/game.cpp:1045](../src/core/game.cpp#L1045)

```cpp
const int pts = 200 << chain_idx;
```

`200 << 0 = 200`, `200 << 1 = 400`, `200 << 2 = 800`, `200 << 3 = 1600`. To change to e.g. start at 100, replace `200` with `100`. To use arbitrary values, replace the formula with a `constexpr int kChainPts[4] = {200, 400, 800, 1600};` array and index it.

### 2.4 Fruit values (default 100, 300, 500, 700, 1000)

**File:** [src/gameplay/fruit.cpp:66-80](../src/gameplay/fruit.cpp#L66-L80).

```cpp
int fruit_points(FruitKind k) {
    switch (k) {
        case FruitKind::Cherry:     return 100;
        case FruitKind::Strawberry: return 300;
        case FruitKind::Orange:     return 500;
        case FruitKind::Apple:      return 700;
        case FruitKind::Melon:      return 1000;
    }
    return 0;
}
```

Change any literal here.

---

# 3. Extra-life threshold

Default fires once at **10 000 points**.

**File:** [src/core/game.cpp](../src/core/game.cpp) — line 70.

```cpp
constexpr int kExtraLifeThreshold = 10000;
```

Replace `10000` with whatever value (e.g. 5000 to make it more accessible).

---

# 4. Pac-Man speed

Default = 6.5 tiles per second.

**File:** [src/core/game.cpp:64](../src/core/game.cpp#L64).

```cpp
constexpr float kBasePacSpeed = 6.5f;
```

Replace with larger value (e.g. 8.0f) to make Pac faster.

> **Note:** The default value is also set on `Pacman::speed_tiles_sec` in [src/gameplay/pacman.h:27](../src/gameplay/pacman.h#L27), but this is overwritten by `apply_level_speed_scaling()` on level load, so editing `game.cpp` is sufficient.

---

# 5. Ghost speed

Default = 5.5 tiles per second.

**File:** [src/core/game.cpp:63](../src/core/game.cpp#L63).

```cpp
constexpr float kBaseGhostSpeed = 5.5f;
```

To make ghosts faster than Pac, set it above 6.5. To make them sluggish (easier game), set it to 4.0 or less.

---

# 6. Frightened (blue-ghost) duration

Default = 6 seconds.

**File:** [src/core/game.cpp:65](../src/core/game.cpp#L65).

```cpp
constexpr float kBaseFrightenedSecs = 6.0f;
```

Replace with 10.0f or whatever the teacher asks.

---

# 7. Per-level difficulty scaling

Three constants — all in [src/core/game.cpp](../src/core/game.cpp) lines 66–68:

```cpp
constexpr float kGhostSpeedPerLevel  = 0.08f;   // ghosts get 8% faster per level
constexpr float kPacSpeedPerLevel    = 0.05f;   // pac gets 5% faster per level
constexpr float kFrightenedDecayPer  = 0.85f;   // frightened time × 0.85 per level
```

Set the first two to `0.0f` to disable per-level speed scaling. Set `kFrightenedDecayPer` to `1.0f` to keep frightened duration constant across levels.

---

# 8. Number of levels

Default = 3.

**File:** [src/core/game.cpp:50-55](../src/core/game.cpp#L50-L55).

```cpp
constexpr int kNumLevels = 3;
constexpr const char* kLevelFiles[kNumLevels] = {
    "assets/levels/level_01.txt",
    "assets/levels/level_02.txt",
    "assets/levels/level_03.txt",
};
```

To bump to N levels:

1. Create `assets/levels/level_0N.txt` (copy of an existing one is fine).
2. Increase `kNumLevels` to N.
3. Add the new entry to `kLevelFiles[]`.
4. Rebuild — the `POST_BUILD` step in CMakeLists.txt will auto-copy the new file.

> See also [§32 — Add a brand-new level](#32-add-a-brand-new-level).

---

# 9. Maze size or tile size

> **⚠ Don't do this during the exam unless asked.** Changing these requires re-authoring every level file. But if you must:

**File:** [src/world/tile.h:9-11](../src/world/tile.h#L9-L11).

```cpp
inline constexpr int kCols = 28;       // number of columns
inline constexpr int kRows = 31;       // number of rows
inline constexpr int kTileSize = 24;   // tile size in pixels
```

Halving `kTileSize` to 12 will shrink the play area to 336×372 — easiest "make it tiny" demo.

---

# 10. Window / aspect ratio

Default window = 672 × 824 px.

**File:** [src/render/gl_init.h:7-8](../src/render/gl_init.h#L7-L8).

```cpp
inline constexpr int kWindowWidth = 672;
inline constexpr int kWindowHeight = 824;
```

These are derived from `kCols*kTileSize` (= 672) and `kRows*kTileSize + kHudHeight` (= 744 + 80). If you change them, the letterbox math will simply rescale — the gameplay area stays internally 672×824, just stretched.

To go widescreen 16:9 (1280×720): the play area will *letterbox* on the top and bottom.

---

# 11. Ghost-house release timings

The four ghosts emerge from the house on a timed schedule:

**File:** [src/gameplay/ghost.cpp:85-116](../src/gameplay/ghost.cpp#L85-L116).

```cpp
case GhostKind::Pinky:  g.house_release_at = 1.0f;  break;   // 1 second
case GhostKind::Inky:   g.house_release_at = 5.0f;  break;   // 5 seconds
case GhostKind::Clyde:  g.house_release_at = 14.0f; break;   // 14 seconds
```

Blinky is hard-coded to start *outside* the house already — see [lines 92-102](../src/gameplay/ghost.cpp#L92-L102). To make Blinky also start inside, replace those lines with the same `InHouse` pattern as Pinky.

To send all four out instantly, set all three to `0.0f`.

---

# 12. Ghost colours

**File:** [src/gameplay/ghost.cpp:39-51](../src/gameplay/ghost.cpp#L39-L51).

```cpp
case GhostKind::Blinky: return {1.00f, 0.10f, 0.10f, 1.0f};  // red
case GhostKind::Pinky:  return {1.00f, 0.72f, 0.85f, 1.0f};  // pink
case GhostKind::Inky:   return {0.10f, 0.85f, 1.00f, 1.0f};  // cyan
case GhostKind::Clyde:  return {1.00f, 0.62f, 0.18f, 1.0f};  // orange
```

Each is `{R, G, B, A}` with components in 0..1. Replace with whatever the teacher asks (e.g. `{0, 1, 0, 1}` for green).

---

# 13. Pac-Man colour or radius

**File:** [src/gameplay/pacman.cpp:25-28](../src/gameplay/pacman.cpp#L25-L28).

```cpp
constexpr float kPacRadius = 11.0f;
constexpr render::Color kPacColor = {1.0f, 0.92f, 0.16f, 1.0f};
```

Change `kPacRadius` to e.g. `15.0f` to make Pac bigger. Note: making him bigger doesn't change collision distance (still 14 px from `collision.cpp`) — see [§17](#17-collision-distance).

---

# 14. Pac-Man mouth animation speed

**File:** [src/gameplay/pacman.cpp:21](../src/gameplay/pacman.cpp#L21).

```cpp
constexpr float kChompHz = 6.0f;   // mouth opens-closes 6 times per second
```

To slow the chomp: `3.0f`. To make him manic: `12.0f`.

---

# 15. Wave (Scatter/Chase) schedule

The seven-phase Scatter/Chase schedule:

**File:** [src/gameplay/modes.cpp:8](../src/gameplay/modes.cpp#L8).

```cpp
constexpr float kPhaseDurations[7] = {7.0f, 20.0f, 7.0f, 20.0f, 5.0f, 20.0f, 5.0f};
//                                    Scat  Chase  Scat  Chase  Scat  Chase  Scat   (then Chase forever)
```

Indices 0,2,4,6 are **Scatter**; indices 1,3,5 are **Chase**. To make ghosts more aggressive, shorten the Scatter phases. To make the game easier (more patrol time), lengthen them.

To make ghosts always chase: set the entire array to all-zeros — they'll skip through all 7 phases on the first frame and lock into Chase forever.

---

# 16. Ghost AI behaviour (personalities)

**The exam favourite.** All four personalities live in one switch statement:

**File:** [src/gameplay/ghost_ai.cpp:51-86](../src/gameplay/ghost_ai.cpp#L51-L86).

```cpp
case GhostKind::Blinky:
    return {pac.col, pac.row};                  // CHANGE TO MAKE BLINKY SMARTER/DUMBER

case GhostKind::Pinky:
    return {pac.col + 4 * util::dx(pac.dir),    // CHANGE THE "4" FOR DIFFERENT LEAD
            pac.row + 4 * util::dy(pac.dir)};

case GhostKind::Inky: {
    const int piv_col = pac.col + 2 * util::dx(pac.dir);  // "2 tiles ahead pivot"
    const int piv_row = pac.row + 2 * util::dy(pac.dir);
    const int dc = piv_col - blinky.col;
    const int dr = piv_row - blinky.row;
    return {piv_col + dc, piv_row + dr};
}

case GhostKind::Clyde: {
    const int dc = g.col - pac.col;
    const int dr = g.row - pac.row;
    if (dc * dc + dr * dr > 64)                 // 64 = 8² tile distance threshold
        return {pac.col, pac.row};
    return scatter_target(GhostKind::Clyde);
}
```

Easy modifications:

- **Make Pinky lead more aggressively:** change `4` to `8`.
- **Make Clyde always chase:** delete the `if (dc*dc+dr*dr > 64) ... else` and just `return {pac.col, pac.row};`.
- **Make all 4 ghosts use the same AI:** replace every case with `return {pac.col, pac.row};`.

---

# 17. Collision distance

Default Pac↔ghost touch threshold = **14 pixels**.

**File:** [src/gameplay/collision.cpp:10-11](../src/gameplay/collision.cpp#L10-L11).

```cpp
constexpr float kTouchPx = 14.0f;
constexpr float kTouchPxSq = kTouchPx * kTouchPx;
```

To make the game more punishing: bump to 18.0f. To make Pac harder to catch: drop to 10.0f.

> **Note:** Fruit and perk pickup use their own inline `14.0f * 14.0f` thresholds, not this constant. Change those at [game.cpp:958](../src/core/game.cpp#L958), [game.cpp:493](../src/core/game.cpp#L493), [game.cpp:697](../src/core/game.cpp#L697) if needed.

---

# 18. Power-pellet visual pulse rate

**File:** [src/core/game.cpp:1506-1507](../src/core/game.cpp#L1506-L1507).

```cpp
const float pulse_t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
const float pulse_r = kPelletRadius + 1.5f * std::sin(pulse_t * 3.6f);
```

- `3.6f` → pulse frequency (in rad/s). Higher = faster pulse.
- `1.5f` → pulse amplitude in pixels. Higher = more dramatic size change.
- `kPelletRadius` is the base radius — set at [game.cpp:45](../src/core/game.cpp#L45) as `6.0f`.

---

# 19. Splash / READY! / Dying / LevelComplete durations

All in [src/core/game.cpp](../src/core/game.cpp), lines 48–90:

```cpp
constexpr float kDyingDuration = 0.9f;         // death animation
constexpr float kLevelCompleteDuration = 1.6f; // celebration before next level
constexpr float kSplashDuration = 1.5f;        // intro fade
constexpr float kReadyDuration = 1.5f;         // "READY!" banner
constexpr float kDoorOpenDuration = 0.5f;      // ghost-house door fade
```

To skip the splash entirely: set `kSplashDuration = 0.0f`. To make death feel impactful: bump `kDyingDuration` to `2.0f`.

---

# 20. Fruit spawn thresholds + spawn tile

### Thresholds (dots eaten before fruit appears)

**File:** [src/core/game.cpp:58](../src/core/game.cpp#L58).

```cpp
constexpr int kFruitSpawnThresholds[] = {70, 170};
```

So fruit spawns after 70 dots AND again after 170 dots. To get a fruit much earlier: `{10, 50}`.

### Spawn tile coordinates

**File:** [src/core/game.cpp:59-60](../src/core/game.cpp#L59-L60).

```cpp
constexpr int kFruitTileCol = 14;
constexpr int kFruitTileRow = 11;
```

### Fruit lifetime on screen

**File:** [src/gameplay/fruit.cpp:11](../src/gameplay/fruit.cpp#L11).

```cpp
constexpr float kFruitLifetime = 9.0f;
```

---

# 21. Perk tuning (durations, spawn timing, speed multiplier)

All in [src/gameplay/perks.h:32-39](../src/gameplay/perks.h#L32-L39):

```cpp
constexpr float kPerkSpawnMin = 25.0f;           // earliest seconds before a perk shows
constexpr float kPerkSpawnMax = 40.0f;           // latest seconds
constexpr float kPerkLifetimeOnMap = 10.0f;      // perk despawns if not picked up

constexpr float kFreezeDuration = 4.0f;          // Freeze active duration
constexpr float kSpeedDuration = 5.0f;           // Speed active duration
constexpr float kInvisibilityDuration = 4.0f;    // Invisibility active duration
constexpr float kSpeedMultiplier = 1.40f;        // Pac speed boost (× 1.40)
```

Want perks to spawn faster: drop `kPerkSpawnMin` and `kPerkSpawnMax`.

Want Speed perk to make Pac twice as fast: `kSpeedMultiplier = 2.0f`.

To disable perks entirely: set `kPerkSpawnMin = 999999.0f`.

---

# 22. Combo / chain multiplier

**File:** [src/gameplay/combo.h:20-23](../src/gameplay/combo.h#L20-L23).

```cpp
constexpr float kComboWindow = 0.40f;         // seconds between eats to keep streak
constexpr float kComboPerLink = 0.10f;        // multiplier added per chain link
constexpr float kComboMax = 3.00f;            // cap on multiplier
constexpr int kComboCapForMax = 21;           // chain length reaching max
```

Easy game: drop `kComboWindow` to `1.0f` (more forgiving) and `kComboPerLink` to `0.05f`. Hard mode: `kComboWindow = 0.20f`.

---

# 23. Blinky fire-burst tuning

**File:** [src/core/game.cpp:73-76](../src/core/game.cpp#L73-L76).

```cpp
constexpr float kBlinkyFireCooldown = 12.0f;   // seconds between bursts
constexpr float kBlinkyTelegraphSec = 0.5f;    // warning halo
constexpr float kBlinkyFiringSec = 0.4f;       // active fire window
constexpr int kBlinkyFireRange = 3;            // tiles ahead of Blinky
```

To disable the fire entirely: set `kBlinkyFireCooldown = 999999.0f`.

To make Blinky terrifying: `kBlinkyFireCooldown = 4.0f` and `kBlinkyFireRange = 6`.

---

# 24. Clyde clone tuning

**File:** [src/core/game.cpp:78-79](../src/core/game.cpp#L78-L79).

```cpp
constexpr float kClydeCloneCooldown = 30.0f;   // seconds between clones
constexpr float kClydeCloneLifetime = 4.0f;    // how long the clone exists
```

To disable: set cooldown to `999999.0f`.

---

# 25. Keyboard bindings

**File:** [src/input/input.cpp:26-103](../src/input/input.cpp#L26-L103).

To remap **W → I** (move up), change line 49 from `case 'w': case 'W':` to `case 'i': case 'I':`.

Notable keys you might be asked to rebind:

| Behaviour | Lines in input.cpp | Default |
|-----------|--------------------|---------|
| Quit      | 31                 | `q`/`Q` |
| Back/Esc  | 35                 | Esc (`27`) |
| Confirm/Enter | 39             | Enter (`13`) |
| Pause     | 43                 | `p`/`P` |
| Move up   | 49-50              | `w`/`W` |
| Move left | 54-55              | `a`/`A` |
| Move down | 59-60              | `s`/`S` |
| Move right| 64-65              | `d`/`D` |
| Full-screen | 97               | F11    |

---

# 26. Game / window title

**File:** [src/core/app.cpp:17](../src/core/app.cpp#L17).

```cpp
constexpr const char* kWindowTitle = "Pac-Man — CSE 426";
```

Replace with whatever the teacher wants in the title bar.

---

# 27. Frame rate / update tick

### Update rate (default 60 Hz)

**File:** [src/core/clock.h:13](../src/core/clock.h#L13).

```cpp
static constexpr int kUpdatesPerSecond = 60;
```

Change to 30 to halve gameplay speed, 120 to double it.

### GLUT timer interval (default 16 ms ≈ 62.5 Hz)

**File:** [src/core/app.cpp:49](../src/core/app.cpp#L49).

```cpp
constexpr unsigned int kFrameIntervalMs = 16;
```

This controls how often `on_timer` fires — render frequency. The fixed-step accumulator handles any mismatch with `kUpdatesPerSecond`.

---

# 28. Camera shake intensity / duration

**File:** [src/core/game.cpp:81-82](../src/core/game.cpp#L81-L82).

```cpp
constexpr float kCameraShakeMagPx = 8.0f;       // pixels of jiggle
constexpr float kCameraShakeDuration = 0.30f;   // seconds of shake
```

Bump to `20.0f` magnitude / `0.8f` duration for a much more dramatic death.

---

# 29. Audio volumes (defaults)

**File:** [src/audio/audio.cpp:268-270](../src/audio/audio.cpp#L268-L270).

```cpp
float g_master_vol = 0.7f;
float g_sfx_vol = 0.8f;
float g_bgm_vol = 0.4f;
```

These are overridden by the save file on load. To force a default of mute: set all to `0.0f` and delete `%APPDATA%/pacman-freeglut/savedata.txt`.

Settings adjustment step size — [src/ui/settings.h](../src/ui/settings.h) — `kAudioStep` controls how much one Left/Right press moves the slider (default 0.05).

---

# 30. Maze / wall / dot colours

**File:** [src/core/game.cpp:37-40](../src/core/game.cpp#L37-L40).

```cpp
constexpr render::Color kWallColor   = {0.13f, 0.13f, 0.87f, 1.0f};  // blue
constexpr render::Color kDoorColor   = {0.95f, 0.72f, 0.85f, 1.0f};  // pink
constexpr render::Color kDotColor    = {1.00f, 0.85f, 0.62f, 1.0f};  // soft yellow
constexpr render::Color kPelletColor = {1.00f, 0.85f, 0.62f, 1.0f};  // soft yellow
```

For "red maze night mode": change `kWallColor` to `{0.7f, 0.1f, 0.1f, 1.0f}`.

---

# 31. Replace / edit a level (the ASCII files)

Level files live in [assets/levels/](../assets/levels/):

- `level_01.txt`
- `level_02.txt`
- `level_03.txt`

Each is exactly **31 rows × 28 columns** of ASCII. Glyphs:

| Glyph    | Meaning                            |
|----------|------------------------------------|
| `#`      | Wall                               |
| `.`      | Dot                                |
| `o`, `O` | Power pellet                       |
| `-`, `_` | Ghost-house door                   |
| (space)  | Empty (walkable, no collectible)   |
| `P`      | Pac-Man spawn (exactly one)        |
| `B`      | Blinky spawn                        |
| `N`      | piNky spawn                         |
| `I`      | Inky spawn                          |
| `C`      | Clyde spawn                         |

**To turn all dots in level_01 into power pellets:** open `assets/levels/level_01.txt`, replace every `.` with `o`.

**To open up the maze (fewer walls):** replace `#` with space in the rows you want clear.

**Rebuild reminder:** the `POST_BUILD` step in CMakeLists.txt re-copies the entire `assets/` folder next to the exe, so just `cmake --build build -j` and the new level is picked up.

> **Tip:** Make a backup of the level file before editing live in the exam!

---

# 32. Add a brand-new level

1. **Create the file:** `assets/levels/level_04.txt`. Copy the contents of `level_01.txt` as a starting point.
2. **Edit `kNumLevels` in game.cpp:**

   **File:** [src/core/game.cpp:50-55](../src/core/game.cpp#L50-L55).

   ```cpp
   constexpr int kNumLevels = 4;                       // was 3
   constexpr const char* kLevelFiles[kNumLevels] = {
       "assets/levels/level_01.txt",
       "assets/levels/level_02.txt",
       "assets/levels/level_03.txt",
       "assets/levels/level_04.txt",                   // NEW
   };
   ```

3. Rebuild — done.

---

# 33. Common "what if it doesn't build" fixes

### "Cannot find freeglut.dll"

The DLL didn't copy. Re-run `cmake --build build -j` (the POST_BUILD step copies it). Or copy `thirdparty/freeglut/bin/libfreeglut.dll` to `build/bin/` manually.

### "level parse error: expected 31 rows, got 30"

You accidentally deleted a row in a level file. Verify exactly 31 lines, each 28 chars wide.

### "level parse error: missing one of P/B/I/N/C spawn markers"

You overwrote a spawn glyph. The level must contain *exactly one* of each of `P`, `B`, `I`, `N`, `C`.

### "level row N col M: bad char 'X'"

Unrecognised glyph in a level file. Only `#`, `.`, `o`, `O`, `-`, `_`, space, `P`, `B`, `I`, `N`, `C` are allowed.

### After changing tile size, Pac falls into walls

`Maze::walkable_for_pac` and `kTileSize` are consistent — but if you changed window dimensions without updating `kCols/kRows`, the maze won't align. Re-verify [tile.h](../src/world/tile.h) and the level files.

### The game compiles but ghosts don't move

You likely set `kGhostSpeedPerLevel` to a negative value or `kBaseGhostSpeed = 0`. Reset to positive.

### Pac-Man teleports / glitches

You set `kBasePacSpeed` too high (> 24 tiles/s ≈ 1 tile per frame at 60 Hz). Anything below 20.0f is safe.

### Build error after deleting a `.cpp` file

The file is listed in [CMakeLists.txt:23-64](../CMakeLists.txt#L23-L64). Remove the matching line from the `PACMAN_SOURCES` list and re-run `cmake -G "MinGW Makefiles" -S . -B build` to refresh the build files.

### Rebuild "doesn't seem to do anything"

You edited a header but the `.cpp` that includes it didn't rebuild. Force a clean rebuild: delete the `build/` directory and re-run the two `cmake` commands.

---

## Top 10 most-likely exam modifications, by probability

Based on the assignment specification — "the teacher will pick a small visible value and ask you to change it" — these are the top 10 to **practice in advance:**

1. **Change lives from 3 to N** → [§1](#1-lives-count-3--4--n)
2. **Change point per dot/pellet** → [§2](#2-point-values-dot-pellet-ghost-fruit)
3. **Make Pac faster / slower** → [§4](#4-pac-man-speed)
4. **Make ghosts faster / slower** → [§5](#5-ghost-speed)
5. **Change frightened time** → [§6](#6-frightened-blue-ghost-duration)
6. **Change ghost colour** → [§12](#12-ghost-colours)
7. **Make Clyde always chase (or always flee)** → [§16](#16-ghost-ai-behaviour-personalities)
8. **Disable Blinky's fire** → [§23](#23-blinky-fire-burst-tuning)
9. **Add a fourth level** → [§32](#32-add-a-brand-new-level)
10. **Skip splash / READY** → [§19](#19-splash--ready--dying--levelcomplete-durations)

For each of these, **practise the edit + rebuild + verify cycle at least once tonight.**

---

## The 60-second emergency drill

If you panic in the exam:

1. <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>F</kbd> in VS Code, type the keyword (e.g. `kStartingLives`).
2. Click the only result. Edit the number.
3. Open a terminal in VS Code. Type `cmake --build build -j` and press Enter.
4. When build succeeds, type `./build/bin/pacman.exe` and press Enter.
5. Demo the change.

You're done in under a minute. Good luck!
