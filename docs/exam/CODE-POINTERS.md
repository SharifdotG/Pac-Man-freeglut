# Code-Pointer Cheat Sheet — Lab Final Exam Prep

> **Use this when the teacher says: *"Show me where the X logic lives."***
>
> Each entry tells you the **file**, the **line range**, the **one-sentence explanation**, and (where useful) a **3-second `grep` to find it live**.
>
> Layout: every clickable link is `[filename:line](path#Lline)` so VS Code jumps you straight there.

---

## Table of contents

1. [Game lifecycle / state machine](#1-game-lifecycle--state-machine)
2. [Win / lose / game-over](#2-win--lose--game-over)
3. [Player (Pac-Man) logic](#3-player-pac-man-logic)
4. [Ghost logic](#4-ghost-logic)
5. [Scoring, lives, hi-score](#5-scoring-lives-hi-score)
6. [Maze / level / tiles](#6-maze--level--tiles)
7. [Collision detection](#7-collision-detection)
8. [Power pellet / frightened mode](#8-power-pellet--frightened-mode)
9. [Fruit](#9-fruit)
10. [Perks (Freeze / Speed / Invisibility)](#10-perks-freeze--speed--invisibility)
11. [Combo / chain multiplier](#11-combo--chain-multiplier)
12. [Special abilities (Blinky fire, Clyde clone)](#12-special-abilities-blinky-fire-clyde-clone)
13. [Input handling](#13-input-handling)
14. [Menu / Help / Settings / Pause / Game-Over screens](#14-menu--help--settings--pause--game-over-screens)
15. [HUD](#15-hud)
16. [Rendering / OpenGL setup](#16-rendering--opengl-setup)
17. [Audio](#17-audio)
18. [Save / load (hi-score, volumes)](#18-save--load-hi-score-volumes)
19. [Visual effects (particles, shake, pulses, fades)](#19-visual-effects-particles-shake-pulses-fades)
20. [Build / entry point](#20-build--entry-point)
21. [Quick keyword cheatsheet (instant grep recipes)](#21-quick-keyword-cheatsheet-instant-grep-recipes)

---

# 1. Game lifecycle / state machine

| What the teacher asks                | File                                                 | Lines     | One-liner |
|--------------------------------------|------------------------------------------------------|-----------|-----------|
| "Show the list of game states"       | [src/core/state.h](../src/core/state.h)              | 8–20      | `enum class GameState` — 11 values: `Splash, Menu, Help, Settings, Ready, Playing, Paused, Dying, LevelComplete, GameOver, WinScreen`. |
| "Show the per-frame state dispatch"  | [src/core/game.cpp](../src/core/game.cpp)            | 1090–1198 | The big `switch (m_state)` in `Game::update`. |
| "Show the per-frame state render dispatch" | [src/core/game.cpp](../src/core/game.cpp)      | 1381–1457 | The big `switch (m_state)` in `Game::render`. |
| "Where do you change states?"        | [src/core/game.cpp](../src/core/game.cpp)            | 267–271   | `Game::enter_state(GameState s)` — resets `m_state_timer` and `m_menu_selection`. |
| "Where does the initial state get set?" | [src/core/game.cpp](../src/core/game.cpp)         | 197–209   | `Game::init()` calls `enter_state(GameState::Splash)`. |
| "Splash duration"                    | [src/core/game.cpp](../src/core/game.cpp)            | 85        | `kSplashDuration = 1.5f`. |
| "READY! banner duration"             | [src/core/game.cpp](../src/core/game.cpp)            | 90        | `kReadyDuration = 1.5f`. |
| "Dying / death animation duration"   | [src/core/game.cpp](../src/core/game.cpp)            | 48        | `kDyingDuration = 0.9f`. |
| "Level-complete celebration duration" | [src/core/game.cpp](../src/core/game.cpp)           | 49        | `kLevelCompleteDuration = 1.6f`. |

**Quick grep:** `enum class GameState`, `enter_state(GameState`, `update_splash`, `update_ready`, `update_playing`.

---

# 2. Win / lose / game-over

### 2.1 Win condition (you-beat-the-level)

[src/core/game.cpp:1083-1087](../src/core/game.cpp#L1083-L1087)

```cpp
if (m_maze->dots_remaining() == 0) {
    audio::stop_bgm();
    enter_state(GameState::LevelComplete);
    std::printf("[pacman] level %d cleared — score=%d\n", m_score.level(), m_score.points());
}
```

### 2.2 Win condition (you-beat-the-whole-game)

After `LevelComplete` timer expires — [src/core/game.cpp:1144-1162](../src/core/game.cpp#L1144-L1162):

```cpp
case GameState::LevelComplete:
    ...
    if (m_state_timer >= kLevelCompleteDuration) {
        const int next = m_level_index + 1;
        if (next >= kNumLevels) {
            enter_state(GameState::WinScreen);   // <-- ALL LEVELS DONE -> YOU WIN
        } else {
            load_level(next);
        }
    }
```

`kNumLevels = 3` is on [src/core/game.cpp:50](../src/core/game.cpp#L50).

### 2.3 Lose condition (you died — last life)

`game_over()` returns true when `lives == 0` — see [src/gameplay/score.h:33-35](../src/gameplay/score.h#L33-L35).

Death routing — [src/core/game.cpp:1120-1142](../src/core/game.cpp#L1120-L1142):

```cpp
case GameState::Dying:
    ...
    if (m_state_timer >= kDyingDuration) {
        if (m_score.game_over()) {
            enter_state(GameState::GameOver);
        } else {
            respawn_actors();
            enter_state(GameState::Ready);
        }
    }
```

### 2.4 Lives counter

- Declaration / `kStartingLives = 3`: [src/gameplay/score.h:10](../src/gameplay/score.h#L10)
- Decrement on hit: `Score::lose_life()` at [src/gameplay/score.cpp:21-25](../src/gameplay/score.cpp#L21-L25)
- Increment on extra-life reward: `Score::add_life()` at [src/gameplay/score.cpp:27-29](../src/gameplay/score.cpp#L27-L29)

### 2.5 The actual "Pac-Man died" call chain

`on_pac_caught()` is the **single entry point** for every kind of death (ghost touch, Blinky fire, Clyde clone):

[src/core/game.cpp:705-717](../src/core/game.cpp#L705-L717)

```cpp
void Game::on_pac_caught() {
    m_score.lose_life();                    // <-- LIFE LOST HERE
    audio::stop_bgm();
    audio::play(audio::SfxId::Death);
    render::particles::emit_dissolve(...);  // explosion
    render::camera::start_shake(...);
    m_active_perk.reset();
    m_clone.active = false;
    enter_state(GameState::Dying);          // <-- DEATH STATE
}
```

### 2.6 Extra-life reward at 10 000 points

[src/core/game.cpp:70](../src/core/game.cpp#L70) — `constexpr int kExtraLifeThreshold = 10000;`

Fires inside `update_playing` — [src/core/game.cpp:1013-1021](../src/core/game.cpp#L1013-L1021):

```cpp
if (!m_extra_life_awarded && score_before < kExtraLifeThreshold &&
    m_score.points() >= kExtraLifeThreshold) {
    m_extra_life_awarded = true;
    m_score.add_life();
    audio::play(audio::SfxId::ExtraLife);
    ...
}
```

### 2.7 Game-over and Win screen rendering

- Game Over panel (red): [src/ui/gameover.cpp:100-102](../src/ui/gameover.cpp#L100-L102)
- Win screen (green): [src/ui/gameover.cpp:104-106](../src/ui/gameover.cpp#L104-L106)
- Both share `draw_screen(...)` at [src/ui/gameover.cpp:34-96](../src/ui/gameover.cpp#L34-L96).

---

# 3. Player (Pac-Man) logic

### 3.1 Pac-Man data structure

[src/gameplay/pacman.h:18-29](../src/gameplay/pacman.h#L18-L29)

```cpp
struct Pacman {
    int col, row;            // current tile
    float offset;            // [0,1) progress into next tile
    util::Direction dir, buffered;
    float anim_time;         // mouth chomp clock
    float speed_tiles_sec;   // default 6.5
    bool alive;
};
```

### 3.2 Pac movement / update logic

`pacman_update()` — [src/gameplay/pacman.cpp:65-153](../src/gameplay/pacman.cpp#L65-L153). Key sub-sections:

- **Input buffer pull** — [line 73](../src/gameplay/pacman.cpp#L73)
- **Instant U-turn** — [lines 77–89](../src/gameplay/pacman.cpp#L77-L89)
- **Start-from-stop** — [lines 92–99](../src/gameplay/pacman.cpp#L92-L99)
- **Advance through tile boundaries (loop)** — [lines 110–150](../src/gameplay/pacman.cpp#L110-L150)
- **Tunnel wrap** — [lines 126–129](../src/gameplay/pacman.cpp#L126-L129)
- **Turn at tile centre** — [lines 134–140](../src/gameplay/pacman.cpp#L134-L140)
- **Stop on wall** — [lines 144–149](../src/gameplay/pacman.cpp#L144-L149)

### 3.3 Pac-Man rendering (procedural wedge / mouth)

`pacman_render()` — [src/gameplay/pacman.cpp:174-203](../src/gameplay/pacman.cpp#L174-L203).

The **chomp animation** is `0.5 - 0.5 * cos(anim_time * 6 Hz * 2π)` — [line 181](../src/gameplay/pacman.cpp#L181).

The **mouth wedge angle** is computed from facing direction by `facing_angle()` at [lines 42–55](../src/gameplay/pacman.cpp#L42-L55).

### 3.4 Pac speed

- **Base value:** `kBasePacSpeed = 6.5f` — [src/core/game.cpp:64](../src/core/game.cpp#L64)
- **Per-level multiplier:** `kPacSpeedPerLevel = 0.05f` — [src/core/game.cpp:67](../src/core/game.cpp#L67)
- **Applied in:** `apply_level_speed_scaling()` — [src/core/game.cpp:396-404](../src/core/game.cpp#L396-L404)
- **Live override (perk):** `update_playing` overrides every frame — [src/core/game.cpp:886-891](../src/core/game.cpp#L886-L891)

### 3.5 Pac spawn position

Read from the level's `P` glyph by `level_loader` → stored in `Maze::spawns().pac_col / pac_row`.

- Glyph parser: [src/world/level_loader.cpp:34-38](../src/world/level_loader.cpp#L34-L38)
- Usage: [src/core/game.cpp:343-348](../src/core/game.cpp#L343-L348)
- In level_01.txt, `P` is at column 13, row 29 (second-from-bottom row).

### 3.6 Tunnel wrap

The tunnel rows in level_01 are the two side-corridors at row 14. Wrap math lives in **every** movement function — search `kCols` for the `col < 0` / `col >= kCols` guard pattern:

- Pac: [src/gameplay/pacman.cpp:30-37](../src/gameplay/pacman.cpp#L30-L37) and [125-129](../src/gameplay/pacman.cpp#L125-L129)
- Ghost: [src/gameplay/ghost.cpp:53-59](../src/gameplay/ghost.cpp#L53-L59) and [170-174](../src/gameplay/ghost.cpp#L170-L174)
- Ghost AI: [src/gameplay/ghost_ai.cpp:19-25](../src/gameplay/ghost_ai.cpp#L19-L25)

---

# 4. Ghost logic

### 4.1 The four ghost kinds

[src/gameplay/ghost.h:10-15](../src/gameplay/ghost.h#L10-L15) — `enum class GhostKind : Blinky, Pinky, Inky, Clyde`.

### 4.2 Ghost data structure

[src/gameplay/ghost.h:27-52](../src/gameplay/ghost.h#L27-L52)

### 4.3 Ghost modes (FSM)

[src/gameplay/ghost.h:17-23](../src/gameplay/ghost.h#L17-L23) — `enum class GhostMode : Chase, Scatter, Frightened, Eaten, InHouse`.

### 4.4 Ghost AI — the personality differences

This is the **single most likely question on the exam.** All four ghost personalities are in **one function** in **one file**:

`chase_target()` — [src/gameplay/ghost_ai.cpp:51-86](../src/gameplay/ghost_ai.cpp#L51-L86).

| Ghost  | Lines  | Behaviour                                                                 |
|--------|--------|---------------------------------------------------------------------------|
| Blinky | 53–55  | `return {pac.col, pac.row};` — direct chase                               |
| Pinky  | 57–61  | 4 tiles ahead of Pac in Pac's facing direction                            |
| Inky   | 63–72  | Pivot 2 tiles ahead of Pac; reflect through Blinky's tile; double-vector  |
| Clyde  | 74–83  | If distance² > 64 → chase directly, else flee to scatter corner            |

### 4.5 Scatter corners (top-right / top-left / bottom-right / bottom-left)

`scatter_target()` — [src/gameplay/ghost_ai.cpp:35-47](../src/gameplay/ghost_ai.cpp#L35-L47):

- Blinky: (25, -3) top-right
- Pinky: (2, -3) top-left
- Inky: (27, 31) bottom-right
- Clyde: (0, 31) bottom-left

### 4.6 The greedy direction picker

`greedy_pick()` — [src/gameplay/ghost_ai.cpp:107-147](../src/gameplay/ghost_ai.cpp#L107-L147). **This is the core AI routine** — for each candidate direction (Up → Left → Down → Right, excluding reverse), it picks the one with the smallest squared distance to the target tile. Strict `<` tie-break preserves the Up-bias.

### 4.7 Frightened-mode random picker

`frightened_pick()` — [src/gameplay/ghost_ai.cpp:151-177](../src/gameplay/ghost_ai.cpp#L151-L177). Uses `std::rand() % n` to pick a uniformly random walkable forward neighbour.

### 4.8 Dispatcher between greedy / frightened

`ai_pick_next_direction()` — [src/gameplay/ghost_ai.cpp:181-190](../src/gameplay/ghost_ai.cpp#L181-L190).

### 4.9 Ghost movement loop (cross-tile, tunnel-wrap)

`ghost_update()` — [src/gameplay/ghost.cpp:118-200](../src/gameplay/ghost.cpp#L118-L200).

Key sub-sections:

- InHouse release timer: [lines 126–138](../src/gameplay/ghost.cpp#L126-L138)
- Frightened countdown: [lines 141–148](../src/gameplay/ghost.cpp#L141-L148)
- Mid-tile reversal: [lines 63–81](../src/gameplay/ghost.cpp#L63-L81) (helper `apply_pending_reverse`)
- Eaten ghost reaches house entrance: [lines 179–184](../src/gameplay/ghost.cpp#L179-L184)
- AI direction pick at each tile boundary: [line 187](../src/gameplay/ghost.cpp#L187)

### 4.10 Per-mode speed multipliers

`speed_multiplier()` — [src/gameplay/ghost.cpp:28-37](../src/gameplay/ghost.cpp#L28-L37):

- Frightened → 0.50× (slower than Pac so player can catch up)
- Eaten → 1.80× (eyes rush home fast)
- Else → 1.00×

### 4.11 Ghost colours

`body_color()` — [src/gameplay/ghost.cpp:39-51](../src/gameplay/ghost.cpp#L39-L51).

- Blinky red `(1.00, 0.10, 0.10)`
- Pinky pink `(1.00, 0.72, 0.85)`
- Inky cyan `(0.10, 0.85, 1.00)`
- Clyde orange `(1.00, 0.62, 0.18)`

### 4.12 Ghost spawn / house release timings

`ghost_init()` — [src/gameplay/ghost.cpp:85-116](../src/gameplay/ghost.cpp#L85-L116):

- Blinky starts already outside (above the house), at tile (13, 11).
- Pinky released at `house_release_at = 1.0` s.
- Inky released at `5.0` s.
- Clyde released at `14.0` s.

### 4.13 Ghost-house entrance tile (where eaten eyes return)

[src/gameplay/ghost_ai.cpp:16-17](../src/gameplay/ghost_ai.cpp#L16-L17) — `kHouseEntryCol = 13, kHouseEntryRow = 11`. Mirrored in [src/gameplay/ghost.cpp:22-23](../src/gameplay/ghost.cpp#L22-L23).

### 4.14 Wave (Scatter/Chase) timer

The single global timer that toggles every non-frightened, non-eaten ghost:

- Schedule: [src/gameplay/modes.cpp:8](../src/gameplay/modes.cpp#L8) — `kPhaseDurations[7] = {7, 20, 7, 20, 5, 20, 5};` followed by Chase-forever.
- Mode lookup by phase: [src/gameplay/modes.cpp:11-13](../src/gameplay/modes.cpp#L11-L13) — even phases = Scatter, odd = Chase.
- Update method: [src/gameplay/modes.cpp:23-39](../src/gameplay/modes.cpp#L23-L39).
- Consumed by `Game::update_playing` to flip every ghost's mode + `pending_reverse`: [src/core/game.cpp:893-905](../src/core/game.cpp#L893-L905).

### 4.15 Ghost rendering (body, eyes, frightened flash, eaten=eyes-only)

`ghost_render()` — [src/gameplay/ghost.cpp:217-271](../src/gameplay/ghost.cpp#L217-L271).

- Frightened white-flash: [lines 227–232](../src/gameplay/ghost.cpp#L227-L232)
- Body suppression when eaten: [line 223](../src/gameplay/ghost.cpp#L223)
- Pupils follow direction: [lines 262–270](../src/gameplay/ghost.cpp#L262-L270)

---

# 5. Scoring, lives, hi-score

| What                                  | Where                                                       |
|---------------------------------------|-------------------------------------------------------------|
| `Score` class definition              | [src/gameplay/score.h](../src/gameplay/score.h)             |
| Starting lives = 3                    | [src/gameplay/score.h:10](../src/gameplay/score.h#L10) — `kStartingLives = 3` |
| Points per dot = 10                   | [src/gameplay/score.h:11](../src/gameplay/score.h#L11) — `kPointsPerDot = 10` |
| Points per pellet = 50                | [src/gameplay/score.h:12](../src/gameplay/score.h#L12) — `kPointsPerPellet = 50` |
| Add points + hi-score auto-update     | [src/gameplay/score.cpp:12-19](../src/gameplay/score.cpp#L12-L19) |
| `lose_life()`                         | [src/gameplay/score.cpp:21-25](../src/gameplay/score.cpp#L21-L25) |
| `add_life()`                          | [src/gameplay/score.cpp:27-29](../src/gameplay/score.cpp#L27-L29) |
| Ghost-eat chain (200, 400, 800, 1600) | [src/core/game.cpp:1043-1046](../src/core/game.cpp#L1043-L1046) — `pts = 200 << chain_idx` |
| Fruit points table                    | [src/gameplay/fruit.cpp:66-80](../src/gameplay/fruit.cpp#L66-L80) |
| Extra life at 10 000                  | [src/core/game.cpp:70](../src/core/game.cpp#L70) + [1013-1021](../src/core/game.cpp#L1013-L1021) |
| Reset for new game                    | [src/gameplay/score.cpp:5-10](../src/gameplay/score.cpp#L5-L10) — hi-score is preserved |

---

# 6. Maze / level / tiles

### 6.1 Maze dimensions

[src/world/tile.h:9-13](../src/world/tile.h#L9-L13):

```cpp
inline constexpr int kCols = 28;
inline constexpr int kRows = 31;
inline constexpr int kTileSize = 24;
inline constexpr int kPlayAreaWidth  = kCols * kTileSize;   // 672
inline constexpr int kPlayAreaHeight = kRows * kTileSize;   // 744
```

### 6.2 Tile types

[src/world/tile.h:17-23](../src/world/tile.h#L17-L23) — `enum class TileType : Empty, Wall, Dot, PowerPellet, GhostDoor`.

### 6.3 Maze class

[src/world/maze.h](../src/world/maze.h):

- `walkable_for_pac()`: [lines 48-51](../src/world/maze.h#L48-L51) — Pac can walk on `Empty/Dot/PowerPellet`
- `walkable_for_ghost()`: [lines 54-56](../src/world/maze.h#L54-L56) — Ghost can additionally cross `GhostDoor`
- `is_wall()`: [lines 61-63](../src/world/maze.h#L61-L63)

### 6.4 `eat_at` — dot/pellet pickup

[src/world/maze.cpp:5-22](../src/world/maze.cpp#L5-L22) — returns 10/50/0 and decrements `m_dots_remaining`.

Called from `update_playing` — [src/core/game.cpp:977](../src/core/game.cpp#L977).

### 6.5 ASCII level parser

[src/world/level_loader.cpp:14-62](../src/world/level_loader.cpp#L14-L62) — `parse_cell()` handles every glyph.

### 6.6 Level files

Three files in [assets/levels/](../assets/levels/):

- `level_01.txt`, `level_02.txt`, `level_03.txt`
- Each is 28 columns × 31 rows of ASCII.

### 6.7 Level switching

In `Game::load_level(int)` — [src/core/game.cpp:333-394](../src/core/game.cpp#L333-L394). Index ≥ `kNumLevels` triggers the WinScreen.

### 6.8 Coordinate conversions

[src/world/tile.h:28-36](../src/world/tile.h#L28-L36):

```cpp
inline int tile_to_px(int t)         { return t * kTileSize; }
inline int tile_center_px(int t)     { return t * kTileSize + kTileSize/2; }
inline int px_to_tile(int px)        { return px / kTileSize; }
```

---

# 7. Collision detection

### 7.1 Pac vs ghost

`pac_ghost_overlap()` — [src/gameplay/collision.cpp:14-22](../src/gameplay/collision.cpp#L14-L22):

```cpp
return (dx*dx + dy*dy) < 14.0f * 14.0f;   // pixel distance, 14 px threshold
```

Called once per ghost per frame in `update_playing` — [src/core/game.cpp:1037-1067](../src/core/game.cpp#L1037-L1067).

### 7.2 Pac vs fruit

Inline in `update_playing` — [src/core/game.cpp:951-970](../src/core/game.cpp#L951-L970):

```cpp
constexpr float kFruitTouchSq = 14.0f * 14.0f;
if (dx_f * dx_f + dy_f * dy_f < kFruitTouchSq) { ... }
```

### 7.3 Pac vs perk

`Game::try_pickup_perk()` — [src/core/game.cpp:484-502](../src/core/game.cpp#L484-L502) — same 14 px threshold.

### 7.4 Pac vs Blinky's fire

`Game::pac_in_blinky_fire()` — [src/core/game.cpp:597-615](../src/core/game.cpp#L597-L615). Walks tile-by-tile from Blinky outward up to `kBlinkyFireRange = 3` tiles, checks if Pac stands on one of them.

### 7.5 Pac vs Clyde clone

`Game::clone_overlaps_pac()` — [src/core/game.cpp:680-699](../src/core/game.cpp#L680-L699). Same 14 px Euclidean test.

### 7.6 Wall collision (movement-time)

Both Pac and ghosts test walkability before crossing into a new tile:

- Pac: `walkable_with_wrap` at [src/gameplay/pacman.cpp:30-37](../src/gameplay/pacman.cpp#L30-L37), used at [line 144](../src/gameplay/pacman.cpp#L144).
- Ghost: same helper at [src/gameplay/ghost.cpp:53-59](../src/gameplay/ghost.cpp#L53-L59), used in ghost_ai's `greedy_pick`.

---

# 8. Power pellet / frightened mode

### 8.1 Power pellet eaten → frightened triggered

[src/core/game.cpp:981-984](../src/core/game.cpp#L981-L984):

```cpp
if (base_pts == gameplay::Score::kPointsPerPellet) {  // 50
    audio::play(audio::SfxId::PowerPellet);
    trigger_frightened();
    m_combo = gameplay::Combo{};
}
```

### 8.2 `trigger_frightened()` — flips every active ghost

[src/core/game.cpp:428-437](../src/core/game.cpp#L428-L437):

```cpp
for (auto& g : m_ghosts) {
    if (g.mode == Eaten || g.mode == InHouse) continue;
    g.mode = GhostMode::Frightened;
    g.mode_timer = m_frightened_duration;
    g.pending_reverse = true;
}
```

### 8.3 Frightened duration per level

- Base: `kBaseFrightenedSecs = 6.0f` — [src/core/game.cpp:65](../src/core/game.cpp#L65)
- Decay factor: `kFrightenedDecayPer = 0.85` per level — [src/core/game.cpp:68](../src/core/game.cpp#L68)
- Computed in `apply_level_speed_scaling` — [src/core/game.cpp:400](../src/core/game.cpp#L400)

### 8.4 Frightened countdown / exit

[src/gameplay/ghost.cpp:141-148](../src/gameplay/ghost.cpp#L141-L148) — when `mode_timer ≤ 0`, revert to wave mode.

### 8.5 Frightened movement uses random picker

[src/gameplay/ghost_ai.cpp:186-188](../src/gameplay/ghost_ai.cpp#L186-L188):

```cpp
if (g.mode == GhostMode::Frightened) {
    return frightened_pick(g, m);
}
```

### 8.6 Frightened rendering (blue + white-flash)

[src/gameplay/ghost.cpp:227-232](../src/gameplay/ghost.cpp#L227-L232) — last 2 s alternate blue/white at ~3 Hz.

---

# 9. Fruit

| What                                  | Where                                                       |
|---------------------------------------|-------------------------------------------------------------|
| `Fruit` struct                        | [src/gameplay/fruit.h](../src/gameplay/fruit.h)             |
| Spawn thresholds (70 / 170 dots)      | [src/core/game.cpp:58](../src/core/game.cpp#L58) — `kFruitSpawnThresholds[] = {70, 170}` |
| Spawn tile (col 14, row 11)           | [src/core/game.cpp:59-60](../src/core/game.cpp#L59-L60)     |
| Spawn trigger                         | [src/core/game.cpp:1023-1032](../src/core/game.cpp#L1023-L1032) — fires when dot-count hits threshold |
| Lifetime on map (9 s)                 | [src/gameplay/fruit.cpp:11](../src/gameplay/fruit.cpp#L11) — `kFruitLifetime = 9.0f` |
| Per-level fruit kind                  | [src/gameplay/fruit.cpp:82-95](../src/gameplay/fruit.cpp#L82-L95) — Cherry→Strawberry→Orange→Apple→Melon |
| Point value table                     | [src/gameplay/fruit.cpp:66-80](../src/gameplay/fruit.cpp#L66-L80) — 100/300/500/700/1000 |
| Pickup logic (collision + score add)  | [src/core/game.cpp:949-970](../src/core/game.cpp#L949-L970) |
| Rendering (body disc + green stem)    | [src/gameplay/fruit.cpp:31-64](../src/gameplay/fruit.cpp#L31-L64) |

---

# 10. Perks (Freeze / Speed / Invisibility)

### 10.1 Perk kinds

[src/gameplay/perks.h:7-11](../src/gameplay/perks.h#L7-L11) — `enum class PerkKind : Freeze, Speed, Invisibility`.

### 10.2 Tunables

[src/gameplay/perks.h:32-39](../src/gameplay/perks.h#L32-L39):

```cpp
constexpr float kPerkSpawnMin = 25.0f;
constexpr float kPerkSpawnMax = 40.0f;
constexpr float kPerkLifetimeOnMap = 10.0f;
constexpr float kFreezeDuration = 4.0f;
constexpr float kSpeedDuration  = 5.0f;
constexpr float kInvisibilityDuration = 4.0f;
constexpr float kSpeedMultiplier = 1.40f;
```

### 10.3 Spawn logic (BFS-derived reachable tiles, ≥3 tile away from Pac)

`Game::update_perk_on_map` — [src/core/game.cpp:443-471](../src/core/game.cpp#L443-L471).

The reachable-tile cache is built once per level by `compute_reachable_tiles()` — [src/core/game.cpp:286-331](../src/core/game.cpp#L286-L331).

### 10.4 Pickup

`Game::try_pickup_perk` — [src/core/game.cpp:484-502](../src/core/game.cpp#L484-L502).

### 10.5 Activation (kind-specific side-effects)

`Game::on_perk_activated` — [src/core/game.cpp:504-531](../src/core/game.cpp#L504-L531):

- Freeze → handled per-frame (`ghosts_frozen` flag skips `ghost_update`)
- Speed → handled per-frame (`pac_speeding` × 1.40 multiplier)
- Invisibility → switches Chase ghosts to Scatter for the duration

### 10.6 Expiry

`Game::on_perk_expired` — [src/core/game.cpp:533-546](../src/core/game.cpp#L533-L546) — Invisibility restores Chase if the wave is Chase.

### 10.7 Per-frame effect application

[src/core/game.cpp:885-891](../src/core/game.cpp#L885-L891) — Pac speed override and ghost-frozen flag.

[src/core/game.cpp:929-936](../src/core/game.cpp#L929-L936) — `if (!ghosts_frozen) ghost_update(...)`.

### 10.8 Visuals

- Pulsing icon on map: [src/core/game.cpp:1313-1320](../src/core/game.cpp#L1313-L1320)
- Screen tint while active: [src/core/game.cpp:1322-1330](../src/core/game.cpp#L1322-L1330)
- HUD timer bar + icon: [src/ui/hud.cpp:103-119](../src/ui/hud.cpp#L103-L119)
- Procedural icons (Freeze snowflake / Speed chevrons / Invis eye-with-slash): [src/gameplay/perks.cpp:53-95](../src/gameplay/perks.cpp#L53-L95)

---

# 11. Combo / chain multiplier

### 11.1 Definition

[src/gameplay/combo.h:15-30](../src/gameplay/combo.h#L15-L30):

```cpp
struct Combo { int chain = 0; float seconds_since_last_eat = 1000.0f; };
constexpr float kComboWindow = 0.40f;
constexpr float kComboPerLink = 0.10f;
constexpr float kComboMax = 3.00f;
```

### 11.2 Multiplier formula

[src/gameplay/combo.h:25-30](../src/gameplay/combo.h#L25-L30):

```cpp
inline float combo_multiplier(const Combo& c) {
    if (c.chain <= 1) return 1.0f;
    const float m = 1.0f + kComboPerLink * (c.chain - 1);
    return (m > kComboMax) ? kComboMax : m;
}
```

### 11.3 On-eat / tick / reset

- `combo_on_eat`: [combo.h:39-46](../src/gameplay/combo.h#L39-L46)
- `combo_tick`: [combo.h:32-37](../src/gameplay/combo.h#L32-L37)
- Reset on power pellet: [game.cpp:984](../src/core/game.cpp#L984) — `m_combo = gameplay::Combo{};`

### 11.4 Floating popups ("+10 ×1.4")

Declared at [combo.h:52-55](../src/gameplay/combo.h#L52-L55), implemented in [src/gameplay/combo.cpp](../src/gameplay/combo.cpp).

Emitted from `update_playing` — [game.cpp:998-1010](../src/core/game.cpp#L998-L1010).

---

# 12. Special abilities (Blinky fire, Clyde clone)

### 12.1 Blinky fire-burst

Three-state mini-FSM inside `Game`:

- Enum: [src/core/game.h:122-127](../src/core/game.h#L122-L127) — `BlinkyAbilState : Cooldown, Telegraph, Firing`.
- Tunables: [src/core/game.cpp:73-76](../src/core/game.cpp#L73-L76) — `kBlinkyFireCooldown=12s`, `kBlinkyTelegraphSec=0.5s`, `kBlinkyFiringSec=0.4s`, `kBlinkyFireRange=3`.
- Update: [src/core/game.cpp:552-595](../src/core/game.cpp#L552-L595).
- Pac-vs-fire collision: [src/core/game.cpp:597-615](../src/core/game.cpp#L597-L615).
- Hit triggers `on_pac_caught()`: [src/core/game.cpp:1069-1074](../src/core/game.cpp#L1069-L1074).
- Render (halo + tile glow): [src/core/game.cpp:1233-1276](../src/core/game.cpp#L1233-L1276).

### 12.2 Clyde clone

A second "ghost" with random AI, spawns every 30 s, lives 4 s:

- Struct: [src/core/game.h:134-141](../src/core/game.h#L134-L141).
- Tunables: [src/core/game.cpp:78-79](../src/core/game.cpp#L78-L79) — `kClydeCloneCooldown=30s`, `kClydeCloneLifetime=4s`.
- Movement (random walk): [src/core/game.cpp:621-678](../src/core/game.cpp#L621-L678).
- Pac collision: [src/core/game.cpp:680-699](../src/core/game.cpp#L680-L699).
- Hit triggers `on_pac_caught()`: [src/core/game.cpp:1076-1081](../src/core/game.cpp#L1076-L1081).
- Render (translucent orange copy): [src/core/game.cpp:1278-1311](../src/core/game.cpp#L1278-L1311).

---

# 13. Input handling

### 13.1 Keyboard map

[src/input/input.cpp:26-103](../src/input/input.cpp#L26-L103):

| Key             | Effect                              |
|-----------------|-------------------------------------|
| Q               | quit immediately                    |
| Esc             | back / cancel                       |
| Enter           | confirm                             |
| P               | pause                               |
| W / A / S / D   | move Up / Left / Down / Right       |
| Arrow keys      | move Up / Left / Down / Right       |
| F11             | toggle full-screen                  |

### 13.2 Input state struct

[src/input/input.h](../src/input/input.h) — `wanted`, `quit_requested`, `press_*` flags.

### 13.3 Edge-triggered flags

`press_enter`, `press_back`, etc. are set on key-down and cleared **once per frame** by `clear_press_flags()` at end of `Game::update` — [src/input/input.cpp:15-24](../src/input/input.cpp#L15-L24) and [src/core/game.cpp:1197](../src/core/game.cpp#L1197).

### 13.4 GLUT wiring

[src/core/app.cpp:115-118](../src/core/app.cpp#L115-L118):

```cpp
glutKeyboardFunc(input::on_keyboard_down);
glutKeyboardUpFunc(input::on_keyboard_up);
glutSpecialFunc(input::on_special_down);
glutSpecialUpFunc(input::on_special_up);
```

### 13.5 `wanted` direction buffer (turn-pre-buffering)

The `wanted` field is *never cleared on key-up* — so a key tapped just before an intersection is remembered until Pac reaches it. See comment at [src/input/input.cpp:74-77](../src/input/input.cpp#L74-L77).

Consumed by `pacman_update` at [src/gameplay/pacman.cpp:73](../src/gameplay/pacman.cpp#L73).

---

# 14. Menu / Help / Settings / Pause / Game-Over screens

### 14.1 Main menu

- Item enum: [src/ui/menu.h:7-13](../src/ui/menu.h#L7-L13) — `Start, Help, Settings, Exit`.
- Render: [src/ui/menu.cpp:58-102](../src/ui/menu.cpp#L58-L102).
- Update / navigation: [src/core/game.cpp:747-782](../src/core/game.cpp#L747-L782).

### 14.2 Help screen

- Render: [src/ui/help.cpp:44-113](../src/ui/help.cpp#L44-L113) — controls table + ghost legend.
- Update (Esc/Enter → back to menu): [src/core/game.cpp:784-790](../src/core/game.cpp#L784-L790).

### 14.3 Settings / audio sliders

- Item enum: [src/ui/settings.h](../src/ui/settings.h) — `Master, Sfx, Bgm`.
- Render: same file's `render_settings_screen`.
- Update / left-right adjustment: [src/core/game.cpp:792-829](../src/core/game.cpp#L792-L829).
- Audio step size: `ui::kAudioStep` in [src/ui/settings.h](../src/ui/settings.h).

### 14.4 Pause

- Item enum: [src/ui/pause.h](../src/ui/pause.h) — `Resume, Menu`.
- Render: [src/ui/pause.cpp:34-75](../src/ui/pause.cpp#L34-L75).
- Update / navigation: [src/core/game.cpp:831-866](../src/core/game.cpp#L831-L866).
- Pause-from-playing trigger: [src/core/game.cpp:870-874](../src/core/game.cpp#L870-L874) (P or Esc).

### 14.5 Game-over / Win

- Item enum: [src/ui/gameover.h](../src/ui/gameover.h) — `Retry, Menu`.
- Render: [src/ui/gameover.cpp:34-106](../src/ui/gameover.cpp#L34-L106).
- Update (handled in big switch): [src/core/game.cpp:1164-1194](../src/core/game.cpp#L1164-L1194).

---

# 15. HUD

### 15.1 HUD layout (bottom 80 px)

[src/ui/hud.cpp:48-119](../src/ui/hud.cpp#L48-L119):

```
| 1UP      |          HIGH SCORE        | LEVEL n |
| {score}  |          {hi_score}        | {level} |
| [lives]  |          TIME  MM:SS       | [perk]  |
```

### 15.2 Lives icons

Mini Pac-Man wedges — `draw_life_icon()` at [src/ui/hud.cpp:27-44](../src/ui/hud.cpp#L27-L44). One drawn per `s.lives()` at [line 86-90](../src/ui/hud.cpp#L86-L90).

### 15.3 Time readout

[src/ui/hud.cpp:92-100](../src/ui/hud.cpp#L92-L100) — `MM:SS` format. The `play_time_seconds` is accumulated only in `Playing` state — [src/core/game.cpp:879-883](../src/core/game.cpp#L879-L883).

### 15.4 Active-perk badge + timer bar

[src/ui/hud.cpp:102-119](../src/ui/hud.cpp#L102-L119).

---

# 16. Rendering / OpenGL setup

### 16.1 GL one-time init

`render::init_gl()` — [src/render/gl_init.cpp:30-41](../src/render/gl_init.cpp#L30-L41):

```cpp
glClearColor(0, 0, 0, 1);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glDisable(GL_DEPTH_TEST);
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
```

### 16.2 Orthographic projection

`apply_logical_projection()` — [src/render/gl_init.cpp:20-26](../src/render/gl_init.cpp#L20-L26):

```cpp
gluOrtho2D(0, kWindowWidth, kWindowHeight, 0);  // top-left origin
```

### 16.3 Letterboxing on reshape / full-screen

`render::on_reshape()` — [src/render/gl_init.cpp:43-70](../src/render/gl_init.cpp#L43-L70). Picks largest centred sub-rect of the new window matching the canonical 672:824 aspect.

### 16.4 Per-frame frame clear

`render::clear_and_apply_viewport()` — [src/render/gl_init.cpp:72-81](../src/render/gl_init.cpp#L72-L81). Two-step: clear full framebuffer (so letterbox bars are black), then restore the letterboxed viewport.

### 16.5 Drawing primitives (the "what teacher writes on the board" stuff)

[src/render/primitives.cpp](../src/render/primitives.cpp):

- `draw_quad` — `GL_QUADS` rectangle: [lines 13-20](../src/render/primitives.cpp#L13-L20)
- `draw_line` — thin quad (not `glLineWidth`, for driver portability): [lines 22-39](../src/render/primitives.cpp#L22-L39)
- `draw_filled_circle` — `GL_TRIANGLE_FAN`: [lines 41-52](../src/render/primitives.cpp#L41-L52)

### 16.6 Text

[src/render/text.cpp:14-21](../src/render/text.cpp#L14-L21) — `glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, …)` per character.

### 16.7 Render layer order (painter's algorithm)

`Game::render()` — [src/core/game.cpp:1381-1457](../src/core/game.cpp#L1381-L1457):

1. `glPushMatrix(); glTranslatef(shake_x, shake_y, 0);`
2. `render_world()` — maze → fruit → perk → Pac → ghosts → clone → blinky-fire → particles → popups
3. `render_screen_tint()`
4. `glPopMatrix();`
5. `ui::render_hud(...)`
6. (state-specific overlay: pause / gameover / win / level-complete banner)

### 16.8 Maze wall outline (edge-aware)

`draw_wall_tile_edges()` — [src/core/game.cpp:94-107](../src/core/game.cpp#L94-L107). Draws only the four edges of a wall tile that face *non-wall* neighbours — this is why walls look like outlines and not filled rectangles.

---

# 17. Audio

### 17.1 SFX identifiers

[src/audio/sfx_ids.h](../src/audio/sfx_ids.h) — `enum class SfxId : Chomp, PowerPellet, EatGhost, EatFruit, Death, ExtraLife, LevelStart, MenuClick, Count`.

### 17.2 Procedural synthesis

[src/audio/audio.cpp](../src/audio/audio.cpp):

- `Synth` class with `add_sine` / `add_square` / `add_sweep` / `add_wobble_square` / `add_silence`: [lines 53-179](../src/audio/audio.cpp#L53-L179).
- `to_wav()` writes a 44-byte RIFF header + PCM-16LE samples: [lines 129-152](../src/audio/audio.cpp#L129-L152).
- Per-SFX patches: [lines 185-241](../src/audio/audio.cpp#L185-L241).
- BGM (4-second wobbling siren): [lines 243-249](../src/audio/audio.cpp#L243-L249).

### 17.3 Engine setup

`audio::init()` — [src/audio/audio.cpp:315-341](../src/audio/audio.cpp#L315-L341). Initialises miniaudio, generates each SFX into memory, loads via `ma_decoder_init_memory`.

### 17.4 Per-track volume

- `set_master_volume`, `set_sfx_volume`, `set_bgm_volume`: [lines 398-423](../src/audio/audio.cpp#L398-L423).
- All multiplied at apply time — [lines 272-283](../src/audio/audio.cpp#L272-L283) — `g_master_vol * g_sfx_vol` etc.

### 17.5 Where each SFX fires

| SFX           | Trigger                                  | Line                                                   |
|---------------|------------------------------------------|--------------------------------------------------------|
| Chomp         | Every dot eaten                          | [game.cpp:989](../src/core/game.cpp#L989)              |
| PowerPellet   | Power pellet eaten / perk activated      | [game.cpp:982](../src/core/game.cpp#L982), [514](../src/core/game.cpp#L514) |
| EatGhost      | Ghost eaten (frightened)                 | [game.cpp:1051](../src/core/game.cpp#L1051)            |
| EatFruit      | Fruit picked up                          | [game.cpp:962](../src/core/game.cpp#L962)              |
| Death         | Pac caught                                | [game.cpp:708](../src/core/game.cpp#L708)              |
| ExtraLife     | Score crosses 10 000                     | [game.cpp:1017](../src/core/game.cpp#L1017)            |
| LevelStart    | Every level load                          | [game.cpp:381](../src/core/game.cpp#L381)              |
| MenuClick     | Menu navigation                          | many places — grep `audio::SfxId::MenuClick`           |

---

# 18. Save / load (hi-score, volumes)

### 18.1 Save location

`%APPDATA%/pacman-freeglut/savedata.txt` — see `util::appdata_path` at [src/util/file.cpp:58-66](../src/util/file.cpp#L58-L66).

### 18.2 Save file format

```
hi_score=12345
master_volume=0.70
sfx_volume=0.80
bgm_volume=0.40
```

### 18.3 Save on exit

`Game::save_user_settings()` — [src/core/game.cpp:253-265](../src/core/game.cpp#L253-L265), called from `core::run_app` after `glutMainLoop` returns — [src/core/app.cpp:136](../src/core/app.cpp#L136).

### 18.4 Load on startup

`Game::load_user_settings()` — [src/core/game.cpp:211-251](../src/core/game.cpp#L211-L251), called from `Game::init` at [line 204](../src/core/game.cpp#L204).

---

# 19. Visual effects (particles, shake, pulses, fades)

### 19.1 Particles

[src/render/particles.cpp](../src/render/particles.cpp):

- `emit_sparkle` (every dot eaten) — [lines 45-62](../src/render/particles.cpp#L45-L62)
- `emit_burst` (ghost eaten, fruit picked up, extra life) — [lines 64-81](../src/render/particles.cpp#L64-L81)
- `emit_ring` (perk activated) — [lines 83-100](../src/render/particles.cpp#L83-L100)
- `emit_dissolve` (Pac death) — [lines 102-120](../src/render/particles.cpp#L102-L120)
- `update` (gravity + lifetime + compact) — [lines 122-138](../src/render/particles.cpp#L122-L138)
- `render` (alpha-fade) — [lines 140-149](../src/render/particles.cpp#L140-L149)

### 19.2 Camera shake

[src/render/camera.cpp](../src/render/camera.cpp):

- `start_shake(mag_px, duration_sec)` — [lines 21-25](../src/render/camera.cpp#L21-L25). Called from `on_pac_caught()` at [game.cpp:712](../src/core/game.cpp#L712).
- `update(dt)` — linear decay to zero — [lines 27-44](../src/render/camera.cpp#L27-L44).
- Applied at render time via `glTranslatef` — [game.cpp:1384-1385](../src/core/game.cpp#L1384-L1385).

### 19.3 Power pellet pulse animation

[src/core/game.cpp:1506-1517](../src/core/game.cpp#L1506-L1517):

```cpp
const float pulse_t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
const float pulse_r = kPelletRadius + 1.5f * std::sin(pulse_t * 3.6f);
```

### 19.4 Selected-menu-item pulse

`pulse_multiplier()` at [src/ui/menu.cpp:40-43](../src/ui/menu.cpp#L40-L43), and identical implementations in [src/ui/pause.cpp:53-54](../src/ui/pause.cpp#L53-L54) and [src/ui/gameover.cpp:70-71](../src/ui/gameover.cpp#L70-L71).

### 19.5 Splash fade-in / fade-out

`Game::render_splash` — [src/core/game.cpp:1342-1379](../src/core/game.cpp#L1342-L1379). Triangular envelope `alpha = t<0.5 ? 2t : 2(1-t)`.

### 19.6 READY! banner wobble

`Game::render_ready_banner` — [src/core/game.cpp:1332-1340](../src/core/game.cpp#L1332-L1340).

### 19.7 Ghost-house door fade

[src/core/game.cpp:1476-1488](../src/core/game.cpp#L1476-L1488). `m_door_open_timer` is set to `kDoorOpenDuration = 0.5f` whenever a ghost exits the house — see triggers at [lines 938-942](../src/core/game.cpp#L938-L942).

---

# 20. Build / entry point

### 20.1 `main()`

[src/main.cpp:8-10](../src/main.cpp#L8-L10):

```cpp
int main(int argc, char** argv) {
    return core::run_app(argc, argv);
}
```

### 20.2 GLUT bootstrap

`core::run_app` — [src/core/app.cpp:88-141](../src/core/app.cpp#L88-L141):

- `glutInit / glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA)` — double buffer
- `glutInitWindowSize(672, 824)`
- `glutCreateWindow("Pac-Man — CSE 426")`
- `render::init_gl()`
- `audio::init()`
- `g_game.init()`
- Wire callbacks
- `glutTimerFunc(16, on_timer, 0)` — 16 ms ≈ 62.5 Hz
- `glutMainLoop()`

### 20.3 Per-frame timer

`on_timer` — [src/core/app.cpp:51-78](../src/core/app.cpp#L51-L78). Calls `g_clock.tick()` and runs that many fixed updates.

### 20.4 CMake build

[CMakeLists.txt](../CMakeLists.txt):

- Sources listed explicitly (no globs): lines 23–64
- Includes: `thirdparty/freeglut/include`, `thirdparty/stb`, `thirdparty/miniaudio` — lines 71–77
- Linking: `freeglut, opengl32, glu32, winmm, gdi32` — lines 89–96
- Post-build asset copy: lines 118–126

---

# 21. Quick keyword cheatsheet (instant grep recipes)

When the teacher asks about a feature mid-exam, type the keyword into VS Code's <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>F</kbd> search. Each row below gives the *single most distinctive* keyword:

| Topic                       | Grep for                  | Lands you in                          |
|-----------------------------|---------------------------|---------------------------------------|
| Win condition               | `dots_remaining() == 0`   | [game.cpp:1083](../src/core/game.cpp#L1083) |
| You-win-the-game            | `WinScreen`               | [game.cpp:1157](../src/core/game.cpp#L1157) |
| Death / life loss           | `on_pac_caught`           | [game.cpp:705](../src/core/game.cpp#L705) |
| Lives counter init          | `kStartingLives`          | [score.h:10](../src/gameplay/score.h#L10) |
| Pac speed                   | `kBasePacSpeed`           | [game.cpp:64](../src/core/game.cpp#L64) |
| Ghost speed                 | `kBaseGhostSpeed`         | [game.cpp:63](../src/core/game.cpp#L63) |
| Frightened duration         | `kBaseFrightenedSecs`     | [game.cpp:65](../src/core/game.cpp#L65) |
| Extra life threshold        | `kExtraLifeThreshold`     | [game.cpp:70](../src/core/game.cpp#L70) |
| Power-pellet trigger        | `trigger_frightened`      | [game.cpp:428](../src/core/game.cpp#L428) |
| Ghost AI personalities      | `chase_target`            | [ghost_ai.cpp:51](../src/gameplay/ghost_ai.cpp#L51) |
| Scatter corners             | `scatter_target`          | [ghost_ai.cpp:35](../src/gameplay/ghost_ai.cpp#L35) |
| Wave schedule               | `kPhaseDurations`         | [modes.cpp:8](../src/gameplay/modes.cpp#L8) |
| Fruit point values          | `fruit_points`            | [fruit.cpp:66](../src/gameplay/fruit.cpp#L66) |
| Score-per-dot               | `kPointsPerDot`           | [score.h:11](../src/gameplay/score.h#L11) |
| Score-per-pellet            | `kPointsPerPellet`        | [score.h:12](../src/gameplay/score.h#L12) |
| Eat-ghost chain (200/400/…) | `200 << chain_idx`        | [game.cpp:1045](../src/core/game.cpp#L1045) |
| Maze size 28×31             | `kCols`                   | [tile.h:9](../src/world/tile.h#L9) |
| Tile size 24                | `kTileSize`               | [tile.h:11](../src/world/tile.h#L11) |
| Window size 672×824         | `kWindowWidth`            | [gl_init.h:7](../src/render/gl_init.h#L7) |
| Frame rate                  | `kUpdatesPerSecond`       | [clock.h:13](../src/core/clock.h#L13) |
| Keyboard map                | `on_keyboard_down`        | [input.cpp:26](../src/input/input.cpp#L26) |
| OpenGL init                 | `init_gl`                 | [gl_init.cpp:30](../src/render/gl_init.cpp#L30) |
| Letterbox / full-screen     | `on_reshape`              | [gl_init.cpp:43](../src/render/gl_init.cpp#L43) |
| Audio synth                 | `Synth`                   | [audio.cpp:53](../src/audio/audio.cpp#L53) |
| Save file                   | `savedata.txt`            | [game.cpp:87](../src/core/game.cpp#L87) |
| Camera shake                | `start_shake`             | [game.cpp:712](../src/core/game.cpp#L712) |
| Particles on death          | `emit_dissolve`           | [game.cpp:709](../src/core/game.cpp#L709) |
| Splash duration             | `kSplashDuration`         | [game.cpp:85](../src/core/game.cpp#L85) |
| READY duration              | `kReadyDuration`          | [game.cpp:90](../src/core/game.cpp#L90) |
| Combo multiplier            | `combo_multiplier`        | [combo.h:25](../src/gameplay/combo.h#L25) |
| Perk durations              | `kFreezeDuration`         | [perks.h:36](../src/gameplay/perks.h#L36) |
| Blinky fire range           | `kBlinkyFireRange`        | [game.cpp:76](../src/core/game.cpp#L76) |
| Clyde clone lifetime        | `kClydeCloneLifetime`     | [game.cpp:79](../src/core/game.cpp#L79) |
| State enum                  | `enum class GameState`    | [state.h:8](../src/core/state.h#L8) |
| Ghost mode enum             | `enum class GhostMode`    | [ghost.h:17](../src/gameplay/ghost.h#L17) |
| Ghost kind enum             | `enum class GhostKind`    | [ghost.h:10](../src/gameplay/ghost.h#L10) |
| Direction enum              | `enum class Direction`    | [direction.h:9](../src/util/direction.h#L9) |
| BFS for perks               | `compute_reachable_tiles` | [game.cpp:286](../src/core/game.cpp#L286) |

---

## Exam-day strategy

1. **The teacher names a feature. Open `CODE-POINTERS.md` (this file). `Ctrl+F` the feature name.**
2. **Click the linked file:line. You jump straight to the code.**
3. **Read the surrounding 10 lines aloud.** That is your answer.

If you forget which file to grep:

- **Anything player-related** → `src/gameplay/pacman.cpp`
- **Anything ghost-related** → `src/gameplay/ghost*.cpp`
- **Anything score-related** → `src/gameplay/score.cpp`
- **Anything UI/menu-related** → `src/ui/*.cpp`
- **Anything render-/OpenGL-related** → `src/render/*.cpp`
- **Anything state-/transition-/game-flow-related** → `src/core/game.cpp` (always)
- **Anything maze-/tile-/level-related** → `src/world/*.cpp`
- **Anything input-related** → `src/input/input.cpp`
- **Anything audio-related** → `src/audio/audio.cpp`

Good luck — you've got this!
