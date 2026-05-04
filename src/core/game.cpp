#include "core/game.h"

#include <GL/freeglut.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

#include "util/file.h"

#if 0
#include "audio/audio.h"
#include "gameplay/collision.h"
#include "input/input.h"
#include "render/camera.h"
#include "render/gl_init.h"
#include "render/particles.h"
#include "render/primitives.h"
#include "render/text.h"
#include "ui/gameover.h"
#include "ui/help.h"
#include "ui/hud.h"
#include "ui/menu.h"
#include "ui/pause.h"
#include "ui/settings.h"
#include "world/level_loader.h"
#include "world/tile.h"
#endif

#if 0
namespace core {

namespace {

// === Maze palette ============================================================
constexpr render::Color kWallColor = {0.13f, 0.13f, 0.87f, 1.0f};
constexpr render::Color kDoorColor = {0.95f, 0.72f, 0.85f, 1.0f};
constexpr render::Color kDotColor = {1.00f, 0.85f, 0.62f, 1.0f};
constexpr render::Color kPelletColor = {1.00f, 0.85f, 0.62f, 1.0f};

constexpr float kWallStroke = 2.5f;
constexpr float kWallInset = 1.0f;
constexpr float kDotRadius = 2.0f;
constexpr float kPelletRadius = 6.0f;

// === Level / state durations ================================================
constexpr float kDyingDuration = 0.9f;
constexpr float kLevelCompleteDuration = 1.6f;
constexpr int kNumLevels = 3;
constexpr const char* kLevelFiles[kNumLevels] = {
    "assets/levels/level_01.txt",
    "assets/levels/level_02.txt",
    "assets/levels/level_03.txt",
};

// === Fruit configuration ====================================================
constexpr int kFruitSpawnThresholds[] = {70, 170};
constexpr int kFruitTileCol = 14;
constexpr int kFruitTileRow = 11;

// === Difficulty scaling per level ===========================================
constexpr float kBaseGhostSpeed = 5.5f;
constexpr float kBasePacSpeed = 6.5f;
constexpr float kBaseFrightenedSecs = 6.0f;
constexpr float kGhostSpeedPerLevel = 0.08f;
constexpr float kPacSpeedPerLevel = 0.05f;
constexpr float kFrightenedDecayPer = 0.85f;

constexpr int kExtraLifeThreshold = 10000;

// === P10 ====================================================================
constexpr float kBlinkyFireCooldown = 12.0f;
constexpr float kBlinkyTelegraphSec = 0.5f;
constexpr float kBlinkyFiringSec = 0.4f;
constexpr int kBlinkyFireRange = 3;

constexpr float kClydeCloneCooldown = 30.0f;
constexpr float kClydeCloneLifetime = 4.0f;

constexpr float kCameraShakeMagPx = 8.0f;
constexpr float kCameraShakeDuration = 0.30f;

// === P11 polish =============================================================
constexpr float kSplashDuration = 1.5f;    // total splash beat
constexpr float kDoorOpenDuration = 0.5f;  // ghost-house door fade-out
constexpr const char* kSettingsFile = "savedata.txt";

// === P12 polish =============================================================
constexpr float kReadyDuration = 1.5f;  // "READY!" beat before play resumes
constexpr int kPerkMinPacDistance = 3;  // tiles — perk shouldn't spawn on top of Pac

// === Wall outline ===========================================================
void draw_wall_tile_edges(const world::Maze& m, int col, int row) {
    const float x0 = static_cast<float>(world::tile_to_px(col)) + kWallInset;
    const float y0 = static_cast<float>(world::tile_to_px(row)) + kWallInset;
    const float x1 = x0 + static_cast<float>(world::kTileSize) - 2.0f * kWallInset;
    const float y1 = y0 + static_cast<float>(world::kTileSize) - 2.0f * kWallInset;
    if (!m.is_wall(col, row - 1))
        render::draw_line(x0, y0, x1, y0, kWallStroke);
    if (!m.is_wall(col, row + 1))
        render::draw_line(x0, y1, x1, y1, kWallStroke);
    if (!m.is_wall(col - 1, row))
        render::draw_line(x0, y0, x0, y1, kWallStroke);
    if (!m.is_wall(col + 1, row))
        render::draw_line(x1, y0, x1, y1, kWallStroke);
}

const char* ghost_name(gameplay::GhostKind k) {
    switch (k) {
        case gameplay::GhostKind::Blinky:
            return "Blinky";
        case gameplay::GhostKind::Pinky:
            return "Pinky";
        case gameplay::GhostKind::Inky:
            return "Inky";
        case gameplay::GhostKind::Clyde:
            return "Clyde";
    }
    return "?";
}

float frand() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

// Pick a uniformly-random walkable tile suitable for a perk spawn.
//
// P12: chooses from the precomputed `reachable` set (BFS-derived from
// Pac's spawn) and excludes any tile within `kPerkMinPacDistance`
// Manhattan tiles of Pac's current position so perks don't appear
// on top of the player. Returns false only if every reachable tile is
// too close to Pac, which is essentially impossible on a 28×31 maze.
bool pick_random_perk_tile(
    const std::vector<int>& reachable, int pac_col, int pac_row, int& out_col, int& out_row) {
    if (reachable.empty())
        return false;
    constexpr int kMaxAttempts = 32;
    for (int i = 0; i < kMaxAttempts; ++i) {
        const int packed =
            reachable[static_cast<std::size_t>(std::rand() % static_cast<int>(reachable.size()))];
        const int c = packed / world::kRows;
        const int r = packed % world::kRows;
        const int dc = c - pac_col;
        const int dr = r - pac_row;
        const int manhattan = (dc < 0 ? -dc : dc) + (dr < 0 ? -dr : dr);
        if (manhattan < kPerkMinPacDistance)
            continue;
        out_col = c;
        out_row = r;
        return true;
    }
    return false;
}

// Frightened-style random direction for the Clyde clone. Mirrors the
// frightened_pick() in ghost_ai.cpp so the clone wanders erratically.
util::Direction pick_clone_direction(const world::Maze& m,
                                     int col,
                                     int row,
                                     util::Direction current) {
    const util::Direction reverse = util::opposite(current);
    constexpr util::Direction kCandidates[4] = {
        util::Direction::Up,
        util::Direction::Left,
        util::Direction::Down,
        util::Direction::Right,
    };
    util::Direction options[4];
    int n = 0;
    for (util::Direction d : kCandidates) {
        if (d == reverse)
            continue;
        int nc = col + util::dx(d);
        int nr = row + util::dy(d);
        if (nc < 0)
            nc += world::kCols;
        else if (nc >= world::kCols)
            nc -= world::kCols;
        if (!m.walkable_for_ghost(nc, nr))
            continue;
        options[n++] = d;
    }
    if (n > 0)
        return options[std::rand() % n];
    if (reverse != util::Direction::None)
        return reverse;
    return util::Direction::None;
}

}  // namespace

// =============================================================================
// Lifecycle
// =============================================================================

bool Game::init() {
    auto maze = world::load_level_file(kLevelFiles[0]);
    if (!maze)
        return false;

    // P11: pull persistent hi-score + volumes from %APPDATA% before the
    // splash so the menu's HIGH SCORE row reflects past games.
    load_user_settings();

    enter_state(GameState::Splash);
    std::printf("[pacman] init OK — entering splash.\n");
    return true;
}

void Game::load_user_settings() {
    const std::string path = util::appdata_path(kSettingsFile);
    auto contents = util::read_text_file(path);
    if (!contents) {
        // First run — nothing to load. Defaults already in place.
        return;
    }
    std::stringstream ss(*contents);
    std::string line;
    while (std::getline(ss, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        const char* cval = val.c_str();
        char* endp = nullptr;
        if (key == "hi_score") {
            const long n = std::strtol(cval, &endp, 10);
            if (endp != cval)
                m_score.set_hi_score(static_cast<int>(n));
        } else if (key == "master_volume") {
            const float f = std::strtof(cval, &endp);
            if (endp != cval)
                audio::set_master_volume(f);
        } else if (key == "sfx_volume") {
            const float f = std::strtof(cval, &endp);
            if (endp != cval)
                audio::set_sfx_volume(f);
        } else if (key == "bgm_volume") {
            const float f = std::strtof(cval, &endp);
            if (endp != cval)
                audio::set_bgm_volume(f);
        }
    }
    std::printf("[pacman] loaded user settings: hi-score=%d  master=%.2f  sfx=%.2f  bgm=%.2f\n",
                m_score.hi_score(),
                audio::master_volume(),
                audio::sfx_volume(),
                audio::bgm_volume());
}

void Game::save_user_settings() {
    std::stringstream ss;
    ss << "hi_score=" << m_score.hi_score() << "\n"
       << "master_volume=" << audio::master_volume() << "\n"
       << "sfx_volume=" << audio::sfx_volume() << "\n"
       << "bgm_volume=" << audio::bgm_volume() << "\n";
    const std::string path = util::appdata_path(kSettingsFile);
    if (!util::write_text_file(path, ss.str())) {
        std::printf("[pacman:WARN] could not save user settings to %s\n", path.c_str());
        return;
    }
    std::printf("[pacman] saved user settings to %s\n", path.c_str());
}

void Game::enter_state(GameState s) {
    m_state = s;
    m_state_timer = 0.0f;
    m_menu_selection = 0;
}

void Game::start_new_game() {
    m_score.reset_for_new_game();
    m_won_logged = false;
    m_game_over_logged = false;
    m_extra_life_awarded = false;
    m_play_time_seconds = 0.0f;
    if (!load_level(0)) {
        std::printf("[pacman:ERROR] could not start new game — load_level failed.\n");
        audio::stop_bgm();
        enter_state(GameState::Menu);
    }
}

void Game::compute_reachable_tiles() {
    m_reachable_tiles.clear();
    if (!m_maze)
        return;
    const auto& s = m_maze->spawns();
    if (s.pac_col < 0 || s.pac_row < 0)
        return;

    // BFS from Pac's spawn through walkable_for_pac neighbours. We use a
    // visited bitmap of size kCols × kRows so the queue can't enqueue
    // the same tile twice.
    std::vector<unsigned char> visited(world::kCols * world::kRows, 0u);
    auto idx = [](int c, int r) { return r * world::kCols + c; };

    std::queue<std::pair<int, int>> q;
    q.emplace(s.pac_col, s.pac_row);
    visited[static_cast<std::size_t>(idx(s.pac_col, s.pac_row))] = 1u;

    while (!q.empty()) {
        const auto [c, r] = q.front();
        q.pop();
        // Pack (col, row) so callers can pull a random tile in O(1) by
        // indexing the vector directly.
        m_reachable_tiles.push_back(c * world::kRows + r);

        constexpr int kDx[4] = {-1, 1, 0, 0};
        constexpr int kDy[4] = {0, 0, -1, 1};
        for (int i = 0; i < 4; ++i) {
            int nc = c + kDx[i];
            int nr = r + kDy[i];
            // Tunnel wrap on the column axis.
            if (nc < 0)
                nc += world::kCols;
            else if (nc >= world::kCols)
                nc -= world::kCols;
            if (nr < 0 || nr >= world::kRows)
                continue;
            if (visited[static_cast<std::size_t>(idx(nc, nr))])
                continue;
            if (!m_maze->walkable_for_pac(nc, nr))
                continue;
            visited[static_cast<std::size_t>(idx(nc, nr))] = 1u;
            q.emplace(nc, nr);
        }
    }
}

bool Game::load_level(int level_index) {
    if (level_index < 0 || level_index >= kNumLevels) {
        enter_state(GameState::WinScreen);
        return true;
    }

    auto maze = world::load_level_file(kLevelFiles[level_index]);
    if (!maze)
        return false;

    const auto& s = maze->spawns();
    m_maze = std::move(*maze);
    m_level_index = level_index;
    m_score.set_level(level_index + 1);

    gameplay::pacman_init(m_pacman, s.pac_col, s.pac_row);
    gameplay::ghost_init(m_ghosts[0], gameplay::GhostKind::Blinky, s.blinky_col, s.blinky_row);
    gameplay::ghost_init(m_ghosts[1], gameplay::GhostKind::Pinky, s.pinky_col, s.pinky_row);
    gameplay::ghost_init(m_ghosts[2], gameplay::GhostKind::Inky, s.inky_col, s.inky_row);
    gameplay::ghost_init(m_ghosts[3], gameplay::GhostKind::Clyde, s.clyde_col, s.clyde_row);
    apply_level_speed_scaling();

    m_wave.reset();
    m_eat_chain = 0;
    m_dots_eaten_this_level = 0;
    m_fruit = gameplay::Fruit{};

    // P10 reset
    m_perk_on_map = gameplay::Perk{};
    m_active_perk.reset();
    m_perk_spawn_cooldown =
        gameplay::kPerkSpawnMin + frand() * (gameplay::kPerkSpawnMax - gameplay::kPerkSpawnMin);
    m_combo = gameplay::Combo{};
    m_blinky_state = BlinkyAbilState::Cooldown;
    m_blinky_state_timer = kBlinkyFireCooldown;
    m_clone = ClydeClone{};
    m_clone_cooldown = kClydeCloneCooldown;
    render::particles::clear();
    render::camera::clear();
    gameplay::popups_clear();

    // P12: precompute Pac-reachable tile set for valid perk spawns.
    compute_reachable_tiles();

    // P12: every level transitions to Ready (1.5s "READY!" beat) before
    // gameplay engages. Replaces the immediate Playing entry.
    enter_state(GameState::Ready);
    input::state().wanted = util::Direction::None;
    audio::play(audio::SfxId::LevelStart);
    // BGM stays stopped during Ready and resumes on Playing entry —
    // matches the arcade's "music silenced during the READY beat".

    std::printf(
        "[pacman] level %d loaded — %d collectibles  pac=%.2f t/s  ghost=%.2f t/s  "
        "frightened=%.1fs\n",
        level_index + 1,
        m_maze->dots_remaining(),
        m_pacman.speed_tiles_sec,
        m_ghosts[0].speed_tiles_sec,
        m_frightened_duration);
    return true;
}

void Game::apply_level_speed_scaling() {
    const float lvl = static_cast<float>(m_level_index);
    const float ghost_mult = 1.0f + kGhostSpeedPerLevel * lvl;
    const float pac_mult = 1.0f + kPacSpeedPerLevel * lvl;
    m_frightened_duration = kBaseFrightenedSecs * std::pow(kFrightenedDecayPer, lvl);
    m_pacman.speed_tiles_sec = kBasePacSpeed * pac_mult;
    for (auto& g : m_ghosts)
        g.speed_tiles_sec = kBaseGhostSpeed * ghost_mult;
}

void Game::respawn_actors() {
    if (!m_maze)
        return;
    const auto& s = m_maze->spawns();
    gameplay::pacman_init(m_pacman, s.pac_col, s.pac_row);
    gameplay::ghost_init(m_ghosts[0], gameplay::GhostKind::Blinky, s.blinky_col, s.blinky_row);
    gameplay::ghost_init(m_ghosts[1], gameplay::GhostKind::Pinky, s.pinky_col, s.pinky_row);
    gameplay::ghost_init(m_ghosts[2], gameplay::GhostKind::Inky, s.inky_col, s.inky_row);
    gameplay::ghost_init(m_ghosts[3], gameplay::GhostKind::Clyde, s.clyde_col, s.clyde_row);
    apply_level_speed_scaling();
    m_wave.reset();
    m_eat_chain = 0;
    m_active_perk.reset();
    m_blinky_state = BlinkyAbilState::Cooldown;
    m_blinky_state_timer = kBlinkyFireCooldown;
    m_clone = ClydeClone{};
    m_clone_cooldown = kClydeCloneCooldown;
    m_combo = gameplay::Combo{};
    input::state().wanted = util::Direction::None;
    audio::start_bgm();
}

void Game::trigger_frightened() {
    m_eat_chain = 0;
    for (auto& g : m_ghosts) {
        if (g.mode == gameplay::GhostMode::Eaten || g.mode == gameplay::GhostMode::InHouse)
            continue;
        g.mode = gameplay::GhostMode::Frightened;
        g.mode_timer = m_frightened_duration;
        g.pending_reverse = true;
    }
}

// =============================================================================
// P10 — perks
// =============================================================================

void Game::update_perk_on_map(float dt) {
    if (!m_maze)
        return;

    if (m_perk_on_map.active) {
        m_perk_on_map.lifetime_remaining -= dt;
        if (m_perk_on_map.lifetime_remaining <= 0.0f) {
            m_perk_on_map.active = false;
        }
        return;
    }

    // No perk currently visible — count down to next spawn.
    m_perk_spawn_cooldown -= dt;
    if (m_perk_spawn_cooldown > 0.0f)
        return;

    int c = 0, r = 0;
    if (!pick_random_perk_tile(m_reachable_tiles, m_pacman.col, m_pacman.row, c, r)) {
        m_perk_spawn_cooldown = 5.0f;  // try again soon
        return;
    }
    m_perk_on_map.active = true;
    m_perk_on_map.col = c;
    m_perk_on_map.row = r;
    const int kind_idx = std::rand() % 3;
    m_perk_on_map.kind = static_cast<gameplay::PerkKind>(kind_idx);
    m_perk_on_map.lifetime_remaining = gameplay::kPerkLifetimeOnMap;
}

void Game::update_active_perk(float dt) {
    if (!m_active_perk)
        return;
    m_active_perk->remaining -= dt;
    if (m_active_perk->remaining <= 0.0f) {
        const auto kind = m_active_perk->kind;
        m_active_perk.reset();
        on_perk_expired(kind);
    }
}

void Game::try_pickup_perk() {
    if (!m_perk_on_map.active)
        return;
    const float fx = static_cast<float>(world::tile_center_px(m_perk_on_map.col));
    const float fy = static_cast<float>(world::tile_center_px(m_perk_on_map.row));
    const float px = gameplay::pacman_x_px(m_pacman);
    const float py = gameplay::pacman_y_px(m_pacman);
    const float dx = fx - px;
    const float dy = fy - py;
    constexpr float kTouchSq = 14.0f * 14.0f;
    if (dx * dx + dy * dy >= kTouchSq)
        return;

    const auto kind = m_perk_on_map.kind;
    m_perk_on_map.active = false;
    m_perk_spawn_cooldown =
        gameplay::kPerkSpawnMin + frand() * (gameplay::kPerkSpawnMax - gameplay::kPerkSpawnMin);
    on_perk_activated(kind);
}

void Game::on_perk_activated(gameplay::PerkKind kind) {
    gameplay::ActivePerk a;
    a.kind = kind;
    a.remaining = gameplay::perk_duration(kind);
    m_active_perk = a;

    // Visuals + audio for the pickup itself.
    render::particles::emit_ring(gameplay::pacman_x_px(m_pacman),
                                 gameplay::pacman_y_px(m_pacman),
                                 gameplay::perk_color(kind));
    audio::play(audio::SfxId::PowerPellet);
    gameplay::popup_emit(gameplay::perk_short_name(kind),
                         gameplay::pacman_x_px(m_pacman),
                         gameplay::pacman_y_px(m_pacman) - 16.0f,
                         gameplay::perk_color(kind));

    // Per-kind side-effects on the rest of the world.
    if (kind == gameplay::PerkKind::Invisibility) {
        for (auto& g : m_ghosts) {
            if (g.mode == gameplay::GhostMode::Chase) {
                g.mode = gameplay::GhostMode::Scatter;
                g.pending_reverse = true;
            }
        }
    }
    std::printf(
        "[pacman] perk activated: %s (%.1fs)\n", gameplay::perk_short_name(kind), a.remaining);
}

void Game::on_perk_expired(gameplay::PerkKind kind) {
    if (kind == gameplay::PerkKind::Invisibility) {
        // Restore Chase if the global wave is currently in Chase.
        if (m_wave.current_mode() == gameplay::WaveMode::Chase) {
            for (auto& g : m_ghosts) {
                if (g.mode == gameplay::GhostMode::Scatter) {
                    g.mode = gameplay::GhostMode::Chase;
                    g.pending_reverse = true;
                }
            }
        }
    }
    std::printf("[pacman] perk expired: %s\n", gameplay::perk_short_name(kind));
}

// =============================================================================
// P10 — Blinky fire-burst
// =============================================================================

void Game::update_blinky_ability(float dt) {
    auto& blinky = m_ghosts[0];
    // Skip while Blinky isn't menacing the player.
    if (blinky.mode == gameplay::GhostMode::Frightened ||
        blinky.mode == gameplay::GhostMode::Eaten || blinky.mode == gameplay::GhostMode::InHouse) {
        return;
    }

    switch (m_blinky_state) {
        case BlinkyAbilState::Cooldown:
            m_blinky_state_timer -= dt;
            if (m_blinky_state_timer <= 0.0f) {
                m_blinky_state = BlinkyAbilState::Telegraph;
                m_blinky_state_timer = kBlinkyTelegraphSec;
                m_blinky_fire_origin_col = blinky.col;
                m_blinky_fire_origin_row = blinky.row;
                m_blinky_fire_dir = blinky.dir;
                audio::play(audio::SfxId::EatGhost);  // re-using sweep as a "warning"
            }
            break;

        case BlinkyAbilState::Telegraph:
            m_blinky_state_timer -= dt;
            if (m_blinky_state_timer <= 0.0f) {
                m_blinky_state = BlinkyAbilState::Firing;
                m_blinky_state_timer = kBlinkyFiringSec;
                // Re-snapshot at the moment of firing so the burst comes out
                // of where Blinky actually is right NOW, not where he was
                // half a second ago.
                m_blinky_fire_origin_col = blinky.col;
                m_blinky_fire_origin_row = blinky.row;
                m_blinky_fire_dir = blinky.dir;
            }
            break;

        case BlinkyAbilState::Firing:
            m_blinky_state_timer -= dt;
            if (m_blinky_state_timer <= 0.0f) {
                m_blinky_state = BlinkyAbilState::Cooldown;
                m_blinky_state_timer = kBlinkyFireCooldown;
            }
            break;
    }
}

bool Game::pac_in_blinky_fire() const {
    if (m_blinky_state != BlinkyAbilState::Firing)
        return false;
    if (!m_maze)
        return false;
    for (int i = 1; i <= kBlinkyFireRange; ++i) {
        int fc = m_blinky_fire_origin_col + i * util::dx(m_blinky_fire_dir);
        int fr = m_blinky_fire_origin_row + i * util::dy(m_blinky_fire_dir);
        if (fc < 0)
            fc += world::kCols;
        else if (fc >= world::kCols)
            fc -= world::kCols;
        if (m_maze->is_wall(fc, fr))
            break;
        if (m_pacman.col == fc && m_pacman.row == fr)
            return true;
    }
    return false;
}

// =============================================================================
// P10 — Clyde clone
// =============================================================================

void Game::update_clyde_clone(float dt) {
    if (!m_maze)
        return;

    auto& clyde = m_ghosts[3];

    if (m_clone.active) {
        m_clone.lifetime_remaining -= dt;
        if (m_clone.lifetime_remaining <= 0.0f) {
            m_clone.active = false;
            return;
        }
        // Move at clyde's speed using random AI.
        float advance = clyde.speed_tiles_sec * dt;
        while (advance > 0.0f) {
            const float to_next = 1.0f - m_clone.offset;
            if (advance < to_next) {
                m_clone.offset += advance;
                advance = 0.0f;
                break;
            }
            advance -= to_next;
            m_clone.col += util::dx(m_clone.dir);
            m_clone.row += util::dy(m_clone.dir);
            m_clone.offset = 0.0f;
            if (m_clone.col < 0)
                m_clone.col += world::kCols;
            else if (m_clone.col >= world::kCols)
                m_clone.col -= world::kCols;
            const auto next = pick_clone_direction(*m_maze, m_clone.col, m_clone.row, m_clone.dir);
            if (next == util::Direction::None) {
                advance = 0.0f;
                break;
            }
            m_clone.dir = next;
        }
        return;
    }

    // Inactive — wait for cooldown.
    m_clone_cooldown -= dt;
    if (m_clone_cooldown > 0.0f)
        return;
    if (clyde.mode == gameplay::GhostMode::InHouse || clyde.mode == gameplay::GhostMode::Eaten) {
        // Don't spawn from inside the house or while Clyde is eyes-only.
        m_clone_cooldown = 2.0f;
        return;
    }
    m_clone.active = true;
    m_clone.col = clyde.col;
    m_clone.row = clyde.row;
    m_clone.offset = clyde.offset;
    m_clone.dir = clyde.dir;
    m_clone.lifetime_remaining = kClydeCloneLifetime;
    m_clone_cooldown = kClydeCloneCooldown;
    audio::play(audio::SfxId::MenuClick);
    std::printf("[pacman] Clyde clone spawned (%.1fs)\n", kClydeCloneLifetime);
}

bool Game::clone_overlaps_pac() const {
    if (!m_clone.active)
        return false;
    float gx = static_cast<float>(world::tile_to_px(m_clone.col)) + world::kTileSize * 0.5f +
               m_clone.offset * static_cast<float>(world::kTileSize) *
                   static_cast<float>(util::dx(m_clone.dir));
    if (gx < 0.0f)
        gx += static_cast<float>(world::kPlayAreaWidth);
    else if (gx >= static_cast<float>(world::kPlayAreaWidth))
        gx -= static_cast<float>(world::kPlayAreaWidth);
    const float gy = static_cast<float>(world::tile_to_px(m_clone.row)) + world::kTileSize * 0.5f +
                     m_clone.offset * static_cast<float>(world::kTileSize) *
                         static_cast<float>(util::dy(m_clone.dir));
    const float px = gameplay::pacman_x_px(m_pacman);
    const float py = gameplay::pacman_y_px(m_pacman);
    const float dx = gx - px;
    const float dy = gy - py;
    constexpr float kTouchSq = 14.0f * 14.0f;
    return dx * dx + dy * dy < kTouchSq;
}

// =============================================================================
// Death — shared path for ghost / fire / clone hits
// =============================================================================

void Game::on_pac_caught() {
    m_score.lose_life();
    audio::stop_bgm();
    audio::play(audio::SfxId::Death);
    render::particles::emit_dissolve(gameplay::pacman_x_px(m_pacman),
                                     gameplay::pacman_y_px(m_pacman),
                                     {1.0f, 0.92f, 0.16f, 1.0f});
    render::camera::start_shake(kCameraShakeMagPx, kCameraShakeDuration);
    m_active_perk.reset();   // any perk dies with you
    m_clone.active = false;  // clear the clone
    enter_state(GameState::Dying);
    std::printf("[pacman] caught — lives=%d  score=%d\n", m_score.lives(), m_score.points());
}

// =============================================================================
// Update
// =============================================================================

void Game::update_splash() {
    auto& in = input::state();
    // Skip the splash on any user input so impatient players can dive in.
    const bool any_press = in.press_enter || in.press_back || in.press_pause || in.press_up ||
                           in.press_down || in.press_left || in.press_right || in.press_fullscreen;
    if (any_press || m_state_timer >= kSplashDuration) {
        enter_state(GameState::Menu);
    }
}

void Game::update_ready() {
    // Identical skip-on-keypress behaviour as the splash. Once the timer
    // elapses (or any input arrives) we drop into Playing and re-engage
    // the BGM that was paused at level-load / death-respawn time.
    auto& in = input::state();
    const bool any_press = in.press_enter || in.press_back || in.press_pause || in.press_up ||
                           in.press_down || in.press_left || in.press_right;
    if (any_press || m_state_timer >= kReadyDuration) {
        m_state = GameState::Playing;
        m_state_timer = 0.0f;
        audio::start_bgm();
    }
}

void Game::update_menu() {
    using ui::MainMenuItem;
    auto& in = input::state();
    const int n = static_cast<int>(MainMenuItem::Count);
    if (in.press_back) {
        in.quit_requested = true;
        return;
    }
    if (in.press_up) {
        m_menu_selection = (m_menu_selection - 1 + n) % n;
        audio::play(audio::SfxId::MenuClick);
    }
    if (in.press_down) {
        m_menu_selection = (m_menu_selection + 1) % n;
        audio::play(audio::SfxId::MenuClick);
    }
    if (in.press_enter) {
        audio::play(audio::SfxId::MenuClick);
        switch (static_cast<MainMenuItem>(m_menu_selection)) {
            case MainMenuItem::Start:
                start_new_game();
                break;
            case MainMenuItem::Help:
                enter_state(GameState::Help);
                break;
            case MainMenuItem::Settings:
                enter_state(GameState::Settings);
                break;
            case MainMenuItem::Exit:
                in.quit_requested = true;
                break;
            default:
                break;
        }
    }
}

void Game::update_help() {
    auto& in = input::state();
    if (in.press_back || in.press_enter) {
        audio::play(audio::SfxId::MenuClick);
        enter_state(GameState::Menu);
    }
}

void Game::update_settings() {
    using ui::SettingsRow;
    auto& in = input::state();
    const int n = static_cast<int>(SettingsRow::Count);
    if (in.press_back || in.press_enter) {
        audio::play(audio::SfxId::MenuClick);
        enter_state(GameState::Menu);
        return;
    }
    if (in.press_up) {
        m_menu_selection = (m_menu_selection - 1 + n) % n;
        audio::play(audio::SfxId::MenuClick);
    }
    if (in.press_down) {
        m_menu_selection = (m_menu_selection + 1) % n;
        audio::play(audio::SfxId::MenuClick);
    }
    auto adjust = [this](float delta) {
        switch (static_cast<SettingsRow>(m_menu_selection)) {
            case SettingsRow::Master:
                audio::set_master_volume(audio::master_volume() + delta);
                break;
            case SettingsRow::Sfx:
                audio::set_sfx_volume(audio::sfx_volume() + delta);
                audio::play(audio::SfxId::MenuClick);
                break;
            case SettingsRow::Bgm:
                audio::set_bgm_volume(audio::bgm_volume() + delta);
                break;
            default:
                break;
        }
    };
    if (in.press_left)
        adjust(-ui::kAudioStep);
    if (in.press_right)
        adjust(ui::kAudioStep);
}

void Game::update_paused() {
    using ui::PauseItem;
    auto& in = input::state();
    const int n = static_cast<int>(PauseItem::Count);
    if (in.press_pause || in.press_back) {
        audio::play(audio::SfxId::MenuClick);
        audio::start_bgm();
        m_state = GameState::Playing;
        m_state_timer = 0.0f;
        return;
    }
    if (in.press_up) {
        m_menu_selection = (m_menu_selection - 1 + n) % n;
        audio::play(audio::SfxId::MenuClick);
    }
    if (in.press_down) {
        m_menu_selection = (m_menu_selection + 1) % n;
        audio::play(audio::SfxId::MenuClick);
    }
    if (in.press_enter) {
        audio::play(audio::SfxId::MenuClick);
        switch (static_cast<PauseItem>(m_menu_selection)) {
            case PauseItem::Resume:
                audio::start_bgm();
                m_state = GameState::Playing;
                m_state_timer = 0.0f;
                break;
            case PauseItem::Menu:
                audio::stop_bgm();
                enter_state(GameState::Menu);
                break;
            default:
                break;
        }
    }
}

void Game::update_playing(float dt) {
    auto& in = input::state();
    if (in.press_pause || in.press_back) {
        audio::play(audio::SfxId::MenuClick);
        audio::stop_bgm();
        enter_state(GameState::Paused);
        return;
    }
    if (!m_maze)
        return;

    // P12: tally gameplay-only time (excludes Splash/Menu/Help/Settings/
    // Pause/Dying/LevelComplete/Ready) for the HUD's TIME readout and the
    // end-screen TIME line. The win condition is "win in the minimum time"
    // so this is a real metric, not just decoration.
    m_play_time_seconds += dt;

    // === Apply per-frame perk effects to actor speeds.
    const bool ghosts_frozen = m_active_perk && m_active_perk->kind == gameplay::PerkKind::Freeze;
    const bool pac_speeding = m_active_perk && m_active_perk->kind == gameplay::PerkKind::Speed;

    const float lvl = static_cast<float>(m_level_index);
    const float base_pac_speed = kBasePacSpeed * (1.0f + kPacSpeedPerLevel * lvl);
    m_pacman.speed_tiles_sec = base_pac_speed * (pac_speeding ? gameplay::kSpeedMultiplier : 1.0f);

    // === Wave timer.
    m_wave.update(dt);
    if (m_wave.consume_just_changed()) {
        for (auto& g : m_ghosts) {
            if (g.mode == gameplay::GhostMode::Frightened || g.mode == gameplay::GhostMode::Eaten ||
                g.mode == gameplay::GhostMode::InHouse)
                continue;
            g.mode = (m_wave.current_mode() == gameplay::WaveMode::Scatter)
                         ? gameplay::GhostMode::Scatter
                         : gameplay::GhostMode::Chase;
            g.pending_reverse = true;
        }
    }

    // === P10 sub-systems
    update_perk_on_map(dt);
    update_active_perk(dt);
    update_blinky_ability(dt);
    update_clyde_clone(dt);
    gameplay::combo_tick(m_combo, dt);
    gameplay::popups_update(dt);
    render::particles::update(dt);
    render::camera::update(dt);

    // === Actor updates.
    const int prev_col = m_pacman.col;
    const int prev_row = m_pacman.row;

    gameplay::pacman_update(m_pacman, dt, *m_maze);

    // P11: track ghost-house exits for the door-fade animation.
    bool was_in_house[4];
    for (int i = 0; i < 4; ++i) {
        was_in_house[i] = (m_ghosts[i].mode == gameplay::GhostMode::InHouse);
    }

    if (!ghosts_frozen) {
        gameplay::ghost_update(
            m_ghosts[0], dt, *m_maze, m_pacman, m_ghosts[0], m_wave.current_mode());
        for (int i = 1; i < 4; ++i) {
            gameplay::ghost_update(
                m_ghosts[i], dt, *m_maze, m_pacman, m_ghosts[0], m_wave.current_mode());
        }
    }

    for (int i = 0; i < 4; ++i) {
        if (was_in_house[i] && m_ghosts[i].mode != gameplay::GhostMode::InHouse) {
            m_door_open_timer = kDoorOpenDuration;
        }
    }
    if (m_door_open_timer > 0.0f) {
        m_door_open_timer -= dt;
        if (m_door_open_timer < 0.0f)
            m_door_open_timer = 0.0f;
    }

    // === Fruit pickup.
    gameplay::fruit_update(m_fruit, dt);
    if (m_fruit.active) {
        const float fx = static_cast<float>(world::tile_center_px(m_fruit.col));
        const float fy = static_cast<float>(world::tile_center_px(m_fruit.row));
        const float px = gameplay::pacman_x_px(m_pacman);
        const float py = gameplay::pacman_y_px(m_pacman);
        const float dx_f = fx - px;
        const float dy_f = fy - py;
        constexpr float kFruitTouchSq = 14.0f * 14.0f;
        if (dx_f * dx_f + dy_f * dy_f < kFruitTouchSq) {
            const int pts = gameplay::fruit_points(m_fruit.kind);
            m_score.add_points(pts);
            audio::play(audio::SfxId::EatFruit);
            render::particles::emit_burst(fx, fy, 12, {1.0f, 0.7f, 0.2f, 1.0f});
            char buf[16];
            std::snprintf(buf, sizeof(buf), "+%d", pts);
            gameplay::popup_emit(buf, fx, fy, {1.0f, 0.85f, 0.30f, 1.0f});
            std::printf("[pacman] ate fruit (+%d) — score=%d\n", pts, m_score.points());
            m_fruit.active = false;
        }
    }

    // === Perk pickup.
    try_pickup_perk();

    // === Dot / power-pellet pickup.
    if (m_pacman.col != prev_col || m_pacman.row != prev_row) {
        const int base_pts = m_maze->eat_at(m_pacman.col, m_pacman.row);
        if (base_pts > 0) {
            const int score_before = m_score.points();
            int awarded = base_pts;
            if (base_pts == gameplay::Score::kPointsPerPellet) {
                audio::play(audio::SfxId::PowerPellet);
                trigger_frightened();
                m_combo = gameplay::Combo{};  // pellet breaks the dot streak
            } else {
                gameplay::combo_on_eat(m_combo);
                const float mult = gameplay::combo_multiplier(m_combo);
                awarded = static_cast<int>(static_cast<float>(base_pts) * mult + 0.5f);
                audio::play(audio::SfxId::Chomp);
            }
            m_score.add_points(awarded);
            ++m_dots_eaten_this_level;

            // Particle sparkle + popup (only once the streak makes things interesting).
            const float px = gameplay::pacman_x_px(m_pacman);
            const float py = gameplay::pacman_y_px(m_pacman);
            render::particles::emit_sparkle(px, py, kDotColor);
            if (m_combo.chain >= 3 || base_pts == gameplay::Score::kPointsPerPellet) {
                char buf[24];
                if (m_combo.chain >= 3 && base_pts != gameplay::Score::kPointsPerPellet) {
                    std::snprintf(buf,
                                  sizeof(buf),
                                  "+%d  x%.1f",
                                  awarded,
                                  gameplay::combo_multiplier(m_combo));
                } else {
                    std::snprintf(buf, sizeof(buf), "+%d", awarded);
                }
                gameplay::popup_emit(buf, px, py - 14.0f, {1.0f, 0.95f, 0.30f, 1.0f});
            }

            // Extra-life reward at 10 000.
            if (!m_extra_life_awarded && score_before < kExtraLifeThreshold &&
                m_score.points() >= kExtraLifeThreshold) {
                m_extra_life_awarded = true;
                m_score.add_life();
                audio::play(audio::SfxId::ExtraLife);
                render::particles::emit_burst(px, py, 16, {1.0f, 1.0f, 0.4f, 1.0f});
                gameplay::popup_emit("EXTRA LIFE!", px, py - 28.0f, {1.0f, 1.0f, 0.5f, 1.0f});
                std::printf("[pacman] extra life — %d lives now\n", m_score.lives());
            }

            for (int threshold : kFruitSpawnThresholds) {
                if (m_dots_eaten_this_level == threshold) {
                    const auto kind = gameplay::fruit_kind_for_level(m_score.level());
                    gameplay::fruit_spawn(m_fruit, kind, kFruitTileCol, kFruitTileRow);
                    std::printf("[pacman] fruit spawned (%d dots eaten, level %d)\n",
                                m_dots_eaten_this_level,
                                m_score.level());
                    break;
                }
            }
        }
    }

    // === Pac↔ghost overlaps.
    for (auto& g : m_ghosts) {
        if (g.mode == gameplay::GhostMode::InHouse || g.mode == gameplay::GhostMode::Eaten)
            continue;
        if (!gameplay::pac_ghost_overlap(m_pacman, g))
            continue;

        if (g.mode == gameplay::GhostMode::Frightened) {
            const int chain_idx = (m_eat_chain < 4) ? m_eat_chain : 3;
            const int pts = 200 << chain_idx;
            m_score.add_points(pts);
            if (m_eat_chain < 4)
                ++m_eat_chain;
            g.mode = gameplay::GhostMode::Eaten;
            g.mode_timer = 0.0f;
            audio::play(audio::SfxId::EatGhost);
            const float gx = gameplay::ghost_x_px(g);
            const float gy = gameplay::ghost_y_px(g);
            render::particles::emit_burst(gx, gy, 14, {0.4f, 0.8f, 1.0f, 1.0f});
            char buf[16];
            std::snprintf(buf, sizeof(buf), "+%d", pts);
            gameplay::popup_emit(buf, gx, gy - 14.0f, {0.7f, 1.0f, 1.0f, 1.0f});
            std::printf("[pacman] ate %s — +%d  (chain %d/4)  score=%d\n",
                        ghost_name(g.kind),
                        pts,
                        m_eat_chain,
                        m_score.points());
        } else {
            on_pac_caught();
            return;
        }
    }

    // === Pac vs Blinky fire.
    if (pac_in_blinky_fire()) {
        std::printf("[pacman] burned by Blinky's fire!\n");
        on_pac_caught();
        return;
    }

    // === Pac vs Clyde clone.
    if (clone_overlaps_pac()) {
        std::printf("[pacman] caught by Clyde clone!\n");
        on_pac_caught();
        return;
    }

    if (m_maze->dots_remaining() == 0) {
        audio::stop_bgm();
        enter_state(GameState::LevelComplete);
        std::printf("[pacman] level %d cleared — score=%d\n", m_score.level(), m_score.points());
    }
}

void Game::update(double dt_seconds) {
    const float dt = static_cast<float>(dt_seconds);
    auto& in = input::state();
    using ui::GameOverItem;

    switch (m_state) {
        case GameState::Splash:
            m_state_timer += dt;
            update_splash();
            break;
        case GameState::Ready:
            m_state_timer += dt;
            update_ready();
            break;
        case GameState::Menu:
            update_menu();
            break;
        case GameState::Help:
            update_help();
            break;
        case GameState::Settings:
            update_settings();
            break;
        case GameState::Paused:
            update_paused();
            break;
        case GameState::Playing:
            update_playing(dt);
            break;

        case GameState::Dying:
            // Particles + popups + camera continue ticking through the
            // death beat so the burst plays out visually.
            render::particles::update(dt);
            render::camera::update(dt);
            gameplay::popups_update(dt);
            m_state_timer += dt;
            if (m_state_timer >= kDyingDuration) {
                if (m_score.game_over()) {
                    if (!m_game_over_logged) {
                        std::printf("[pacman] GAME OVER — score=%d\n", m_score.points());
                        m_game_over_logged = true;
                    }
                    enter_state(GameState::GameOver);
                } else {
                    // P12: route through Ready (1.5s "READY!" beat) so the
                    // player isn't dropped straight back into a hostile
                    // maze the instant the death animation ends.
                    respawn_actors();
                    enter_state(GameState::Ready);
                }
            }
            break;

        case GameState::LevelComplete:
            render::particles::update(dt);
            gameplay::popups_update(dt);
            m_state_timer += dt;
            if (m_state_timer >= kLevelCompleteDuration) {
                const int next = m_level_index + 1;
                if (next >= kNumLevels) {
                    if (!m_won_logged) {
                        std::printf("[pacman] WIN — all %d levels cleared, score=%d\n",
                                    kNumLevels,
                                    m_score.points());
                        m_won_logged = true;
                    }
                    enter_state(GameState::WinScreen);
                } else {
                    load_level(next);
                }
            }
            break;

        case GameState::GameOver:
        case GameState::WinScreen: {
            const int n = static_cast<int>(GameOverItem::Count);
            if (in.press_back) {
                audio::play(audio::SfxId::MenuClick);
                enter_state(GameState::Menu);
                break;
            }
            if (in.press_up) {
                m_menu_selection = (m_menu_selection - 1 + n) % n;
                audio::play(audio::SfxId::MenuClick);
            }
            if (in.press_down) {
                m_menu_selection = (m_menu_selection + 1) % n;
                audio::play(audio::SfxId::MenuClick);
            }
            if (in.press_enter) {
                audio::play(audio::SfxId::MenuClick);
                switch (static_cast<GameOverItem>(m_menu_selection)) {
                    case GameOverItem::Retry:
                        start_new_game();
                        break;
                    case GameOverItem::Menu:
                        enter_state(GameState::Menu);
                        break;
                    default:
                        break;
                }
            }
            break;
        }
    }

    input::clear_press_flags();
}

// =============================================================================
// Render
// =============================================================================

void Game::render_world() {
    render_maze();
    if (m_perk_on_map.active) {
        render_perk_on_map();
    }
    if (m_fruit.active) {
        gameplay::fruit_render(m_fruit);
    }

    const bool hide_pac = m_state == GameState::Dying && m_state_timer >= kDyingDuration * 0.5f;
    if (!hide_pac) {
        gameplay::pacman_render(m_pacman);
    }

    if (m_state != GameState::Dying && m_state != GameState::LevelComplete) {
        for (const auto& g : m_ghosts) {
            gameplay::ghost_render(g);
        }
        if (m_clone.active)
            render_clyde_clone();
        render_blinky_fire();
    }

    // Particles + popups are part of the world layer so the screen tint
    // and HUD draw on top of them.
    render::particles::render();
    gameplay::popups_render();
}

void Game::render_blinky_fire() const {
    if (m_blinky_state == BlinkyAbilState::Cooldown)
        return;
    const auto& blinky = m_ghosts[0];
    if (blinky.mode == gameplay::GhostMode::Frightened ||
        blinky.mode == gameplay::GhostMode::Eaten || blinky.mode == gameplay::GhostMode::InHouse) {
        return;
    }

    if (m_blinky_state == BlinkyAbilState::Telegraph) {
        // Pulse a translucent red halo around Blinky.
        const float t = m_blinky_state_timer / kBlinkyTelegraphSec;  // 1 → 0
        const float alpha = 0.40f * (1.0f - t);                      // 0 → 0.4
        const float bx = gameplay::ghost_x_px(blinky);
        const float by = gameplay::ghost_y_px(blinky);
        render::set_color({1.0f, 0.30f, 0.20f, alpha});
        render::draw_filled_circle(bx, by, 18.0f, 18);
        return;
    }

    // Firing — render fire tiles in front of the snapshot origin.
    const float life = m_blinky_state_timer / kBlinkyFiringSec;  // 1 → 0
    const float core_a = 0.85f * life;
    if (!m_maze)
        return;
    for (int i = 1; i <= kBlinkyFireRange; ++i) {
        int fc = m_blinky_fire_origin_col + i * util::dx(m_blinky_fire_dir);
        int fr = m_blinky_fire_origin_row + i * util::dy(m_blinky_fire_dir);
        if (fc < 0)
            fc += world::kCols;
        else if (fc >= world::kCols)
            fc -= world::kCols;
        if (m_maze->is_wall(fc, fr))
            break;
        const float cx = static_cast<float>(world::tile_center_px(fc));
        const float cy = static_cast<float>(world::tile_center_px(fr));
        // Outer glow
        render::set_color({1.0f, 0.40f, 0.10f, core_a * 0.5f});
        render::draw_filled_circle(cx, cy, 13.0f, 14);
        // Core
        render::set_color({1.0f, 0.85f, 0.20f, core_a});
        render::draw_filled_circle(cx, cy, 7.0f, 12);
    }
}

void Game::render_clyde_clone() const {
    // Render same shape as Clyde but at half alpha so the clone reads as
    // "ghostly".
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float kRadius = 11.0f;

    float cx = static_cast<float>(world::tile_to_px(m_clone.col)) + world::kTileSize * 0.5f +
               m_clone.offset * static_cast<float>(world::kTileSize) *
                   static_cast<float>(util::dx(m_clone.dir));
    if (cx < 0.0f)
        cx += static_cast<float>(world::kPlayAreaWidth);
    else if (cx >= static_cast<float>(world::kPlayAreaWidth))
        cx -= static_cast<float>(world::kPlayAreaWidth);
    const float cy = static_cast<float>(world::tile_to_px(m_clone.row)) + world::kTileSize * 0.5f +
                     m_clone.offset * static_cast<float>(world::kTileSize) *
                         static_cast<float>(util::dy(m_clone.dir));

    const float fade = m_clone.lifetime_remaining / kClydeCloneLifetime;  // 1 → 0
    const float a = 0.30f + 0.40f * fade;
    render::set_color({1.0f, 0.62f, 0.18f, a});
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    constexpr int kSeg = 18;
    for (int i = 0; i <= kSeg; ++i) {
        const float t = -kPi + (kPi * static_cast<float>(i) / static_cast<float>(kSeg));
        glVertex2f(cx + std::cos(t) * kRadius, cy + std::sin(t) * kRadius);
    }
    glEnd();
    render::draw_quad(cx - kRadius, cy, kRadius * 2.0f, kRadius);

    render::set_color({1.0f, 1.0f, 1.0f, a});
    render::draw_filled_circle(cx - 4.0f, cy - 3.0f, 2.5f, 12);
    render::draw_filled_circle(cx + 4.0f, cy - 3.0f, 2.5f, 12);
}

void Game::render_perk_on_map() const {
    const float cx = static_cast<float>(world::tile_center_px(m_perk_on_map.col));
    const float cy = static_cast<float>(world::tile_center_px(m_perk_on_map.row));
    // Pulse the icon so it stands out.
    const float t = m_perk_on_map.lifetime_remaining;
    const float pulse = 1.0f + 0.10f * std::sin(t * 6.0f);
    gameplay::perk_draw_icon(cx, cy, 9.0f * pulse, m_perk_on_map.kind);
}

void Game::render_screen_tint() const {
    if (!m_active_perk)
        return;
    render::set_color(gameplay::perk_screen_tint(m_active_perk->kind));
    render::draw_quad(0.0f,
                      0.0f,
                      static_cast<float>(world::kPlayAreaWidth),
                      static_cast<float>(world::kPlayAreaHeight));
}

void Game::render_ready_banner() const {
    // Pulse the alpha gently so the banner feels alive instead of static.
    const float t = m_state_timer / kReadyDuration;           // 0 → 1
    const float pulse = 0.85f + 0.15f * std::sin(t * 18.0f);  // ~3 Hz wobble
    const float cx = static_cast<float>(world::kPlayAreaWidth) * 0.5f;
    const float cy = static_cast<float>(world::kPlayAreaHeight) * 0.5f;
    render::set_color({1.0f, 0.92f, 0.16f, pulse});
    render::draw_text_centered(cx, cy, "READY!");
}

void Game::render_splash() const {
    const float t = m_state_timer / kSplashDuration;  // 0 → 1
    // Triangular envelope: fade in for first half, fade out for second.
    const float alpha = (t < 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
    const float a = std::min(std::max(alpha, 0.0f), 1.0f);

    const float cx = static_cast<float>(render::kWindowWidth) * 0.5f;
    const float cy_title = 320.0f;
    const float cy_sub = cy_title + 36.0f;

    // Yellow Pac-Man wedge to the left of the title (just for flair).
    constexpr float kWedgeR = 28.0f;
    constexpr float kPi = 3.14159265358979323846f;
    const float wcx = cx - 130.0f;
    const float wcy = cy_title - 6.0f;
    render::set_color({1.0f, 0.92f, 0.16f, a});
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(wcx, wcy);
    constexpr int seg = 28;
    const float mouth = 0.45f;
    const float arc_start = 0.0f + mouth;
    const float arc_end = 2.0f * kPi - mouth;
    for (int i = 0; i <= seg; ++i) {
        const float th =
            arc_start + (arc_end - arc_start) * static_cast<float>(i) / static_cast<float>(seg);
        glVertex2f(wcx + std::cos(th) * kWedgeR, wcy + std::sin(th) * kWedgeR);
    }
    glEnd();

    render::set_color({1.0f, 0.92f, 0.16f, a});
    render::draw_text_centered(cx + 30.0f, cy_title, "PAC-MAN");

    render::set_color({0.65f, 0.65f, 0.85f, a});
    render::draw_text_centered(cx, cy_sub, "freeglut + C++17  —  CSE 426");

    render::set_color({0.50f, 0.50f, 0.65f, a * 0.8f});
    render::draw_text_centered(cx, 700.0f, "press any key to skip");
}

void Game::render() {
    // Screen-shake offset is applied to the world layer but NOT to the
    // HUD/menus — those should stay legible during the death beat.
    glPushMatrix();
    glTranslatef(render::camera::offset_x(), render::camera::offset_y(), 0.0f);

    switch (m_state) {
        case GameState::Splash:
            glPopMatrix();
            render_splash();
            return;

        case GameState::Menu:
            glPopMatrix();
            ui::render_main_menu(m_menu_selection, m_score.hi_score());
            return;

        case GameState::Help:
            glPopMatrix();
            ui::render_help_screen();
            return;

        case GameState::Settings:
            glPopMatrix();
            ui::render_settings_screen(m_menu_selection);
            return;

        case GameState::Ready:
            // Same world-frozen scene as Paused, but with a yellow READY!
            // banner instead of the pause menu.
            render_world();
            glPopMatrix();
            ui::render_hud(m_score, m_active_perk, m_play_time_seconds);
            render_ready_banner();
            return;

        case GameState::Playing:
        case GameState::Dying:
        case GameState::LevelComplete:
            render_world();
            render_screen_tint();
            glPopMatrix();
            ui::render_hud(m_score, m_active_perk, m_play_time_seconds);
            if (m_state == GameState::LevelComplete) {
                const float cx = static_cast<float>(world::kPlayAreaWidth) * 0.5f;
                const float cy = static_cast<float>(world::kPlayAreaHeight) * 0.5f;
                render::set_color({1.0f, 0.95f, 0.30f, 1.0f});
                char buf[32];
                std::snprintf(buf, sizeof(buf), "LEVEL %d CLEAR!", m_score.level());
                render::draw_text_centered(cx, cy, buf);
            }
            return;

        case GameState::Paused:
            render_world();
            render_screen_tint();
            glPopMatrix();
            ui::render_hud(m_score, m_active_perk, m_play_time_seconds);
            ui::render_pause_overlay(m_menu_selection);
            return;

        case GameState::GameOver:
            render_world();
            glPopMatrix();
            ui::render_hud(m_score, m_active_perk, m_play_time_seconds);
            ui::render_game_over(m_menu_selection, m_score.points(), m_play_time_seconds);
            return;

        case GameState::WinScreen:
            render_world();
            glPopMatrix();
            ui::render_hud(m_score, m_active_perk, m_play_time_seconds);
            ui::render_win_screen(m_menu_selection, m_score.points(), m_play_time_seconds);
            return;
    }
    glPopMatrix();  // unreachable, just satisfies static analysis
}

void Game::render_maze() {
    if (!m_maze)
        return;
    const world::Maze& m = *m_maze;

    render::set_color(kWallColor);
    for (int row = 0; row < world::kRows; ++row) {
        for (int col = 0; col < world::kCols; ++col) {
            if (m.at(col, row) == world::TileType::Wall) {
                draw_wall_tile_edges(m, col, row);
            }
        }
    }

    // P11: door fades out for ~0.5s on a ghost release. `m_door_open_timer`
    // counts down from kDoorOpenDuration to 0 — at full timer the door is
    // ~invisible, at 0 the door is back to its normal pink line.
    const float door_alpha =
        (m_door_open_timer > 0.0f) ? (1.0f - m_door_open_timer / kDoorOpenDuration) : 1.0f;
    render::set_color({kDoorColor.r, kDoorColor.g, kDoorColor.b, door_alpha});
    for (int row = 0; row < world::kRows; ++row) {
        for (int col = 0; col < world::kCols; ++col) {
            if (m.at(col, row) == world::TileType::GhostDoor) {
                const float x = static_cast<float>(world::tile_to_px(col));
                const float y =
                    static_cast<float>(world::tile_to_px(row)) + world::kTileSize * 0.5f - 1.5f;
                render::draw_quad(x, y, static_cast<float>(world::kTileSize), 3.0f);
            }
        }
    }

    render::set_color(kDotColor);
    for (int row = 0; row < world::kRows; ++row) {
        for (int col = 0; col < world::kCols; ++col) {
            if (m.at(col, row) == world::TileType::Dot) {
                const float cx = static_cast<float>(world::tile_center_px(col));
                const float cy = static_cast<float>(world::tile_center_px(row));
                render::draw_quad(
                    cx - kDotRadius, cy - kDotRadius, kDotRadius * 2.0f, kDotRadius * 2.0f);
            }
        }
    }

    // P11: animated pulse so power pellets read as "powerful" instead of
    // identical to fat dots. Sin oscillation at ~0.6 Hz, ±1.5 px around
    // the base radius. `glutGet(GLUT_ELAPSED_TIME)` is fine here; the
    // animation is purely visual and doesn't need fixed-step sync.
    const float pulse_t = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) * 0.001f;
    const float pulse_r = kPelletRadius + 1.5f * std::sin(pulse_t * 3.6f);
    render::set_color(kPelletColor);
    for (int row = 0; row < world::kRows; ++row) {
        for (int col = 0; col < world::kCols; ++col) {
            if (m.at(col, row) == world::TileType::PowerPellet) {
                const float cx = static_cast<float>(world::tile_center_px(col));
                const float cy = static_cast<float>(world::tile_center_px(row));
                render::draw_filled_circle(cx, cy, pulse_r);
            }
        }
    }
}

}  // namespace core
#endif // 0

namespace core {

bool Game::init() {
    enter_state(GameState::Splash);
    // TODO: stub
    return true;
}

void Game::update(double dt_seconds) {
    m_state_timer += static_cast<float>(dt_seconds);
    // TODO: stub
}

void Game::render() {
    // TODO: stub
}

void Game::save_user_settings() {
    // TODO: stub
}

void Game::enter_state(GameState s) {
    m_state = s;
    m_state_timer = 0.0f;
    m_menu_selection = 0;
}

} // namespace core