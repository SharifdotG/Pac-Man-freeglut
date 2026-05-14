#pragma once

#include <array>
#include <optional>
#include <vector>

#include "core/state.h"

#include "gameplay/ghost.h"
#include "gameplay/modes.h"
#include "gameplay/pacman.h"
#include "gameplay/score.h"

#include "util/direction.h"
#include "world/maze.h"

namespace core {

class Game {
  public:
    bool init();
    void update(double dt_seconds);
    void render();

    // Persists hi-score + volume preferences to disk. Called from app
    // shutdown so the next launch picks them up.
    void save_user_settings();

  private:
    // === transitions
    bool load_level(int level_index);
    void start_new_game();
    void enter_state(GameState s);
    void apply_level_speed_scaling();
    void load_user_settings();

    // === per-state update helpers
    void update_splash();
    void update_ready();
    void update_menu();
    void update_help();
    void update_settings();
    void update_paused();
    void update_playing(float dt);

    // === P10 sub-systems
    void update_perk_on_map(float dt);
    void update_active_perk(float dt);
    void try_pickup_perk();

#if 0
    void on_perk_activated(gameplay::PerkKind kind);
    void on_perk_expired(gameplay::PerkKind kind);
#endif

    void update_blinky_ability(float dt);
    void update_clyde_clone(float dt);
    bool pac_in_blinky_fire() const;
    bool clone_overlaps_pac() const;
    void on_pac_caught();

    // === per-state render helpers
    void render_world();
    void render_maze();
    void render_blinky_fire() const;
    void render_clyde_clone() const;
    void render_perk_on_map() const;
    void render_screen_tint() const;

    void respawn_actors();
    void trigger_frightened();

    // === P11/P12 polish helpers
    void render_splash() const;
    void render_ready_banner() const;

    // P12: BFS-derived list of all maze tiles Pac can physically reach
    // from his spawn. Computed once at level load; powers perk-spawn
    // tile selection so perks never appear in a wall, the ghost-house,
    // or any other Pac-unreachable region.
    void compute_reachable_tiles();

    std::optional<world::Maze> m_maze;

    gameplay::Pacman m_pacman;
    std::array<gameplay::Ghost, 4> m_ghosts;
    gameplay::WaveTimer m_wave;
    gameplay::Score m_score;

    GameState m_state = GameState::Splash;
    float m_state_timer = 0.0f;

    // P11: brief fade-out on the ghost-house door whenever a ghost is
    // released. Counts DOWN from kDoorOpenDuration to 0; rendered as the
    // door's alpha. 0 → fully closed (default look), 1 → fully invisible.
    float m_door_open_timer = 0.0f;

    int m_menu_selection = 0;

    int m_level_index = 0;
    int m_dots_eaten_this_level = 0;
    float m_frightened_duration = 6.0f;
    int m_eat_chain = 0;

    // P12: elapsed gameplay time (seconds), counting only while in the
    // Playing state. Shown in the HUD as MM:SS and on the end screens.
    // Reset on start_new_game.
    float m_play_time_seconds = 0.0f;

// === P10: perks ==========================================================
#if 0
    gameplay::Perk m_perk_on_map; // visible pickup
    std::optional<gameplay::ActivePerk>
    m_active_perk;                   // current effect (one at a time)
#endif
    float m_perk_spawn_cooldown = 30.0f; // seconds until next try

    // P12: tiles reachable by Pac from his spawn. Filled by
    // compute_reachable_tiles() at level load. Each entry is a packed
    // (col * kRows + row) so we don't need to build a std::pair vector.
    std::vector<int> m_reachable_tiles;

// === P10: combo / popups =================================================
#if 0
    gameplay::Combo m_combo;
#endif

    // === P10: Blinky fire-burst ability =====================================
    enum class BlinkyAbilState : unsigned char {
        Cooldown,  // counting down
        Telegraph, // 0.5s warning flash
        Firing,    // 0.4s active fire
    };
    BlinkyAbilState m_blinky_state = BlinkyAbilState::Cooldown;
    float m_blinky_state_timer = 12.0f; // initial cooldown
    int m_blinky_fire_origin_col = 0;
    int m_blinky_fire_origin_row = 0;
    util::Direction m_blinky_fire_dir = util::Direction::Left;

    // === P10: Clyde clone ====================================================
    struct ClydeClone {
        bool active = false;
        int col = 0;
        int row = 0;
        float offset = 0.0f;
        util::Direction dir = util::Direction::Left;
        float lifetime_remaining = 0.0f;
    };
    ClydeClone m_clone;
    float m_clone_cooldown = 30.0f;

    // 10000-extra-life reward fires at most once per game.
    bool m_extra_life_awarded = false;

    bool m_won_logged = false;
    bool m_game_over_logged = false;
};

} // namespace core
