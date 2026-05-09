#pragma once

namespace gameplay {

// All scoring + life + level state lives here. The HUD reads it; gameplay
// modules write to it via the small mutator API below. There is no event
// system — callers just call add_points / lose_life directly.
class Score {
   public:
    static constexpr int kStartingLives = 3;
    static constexpr int kPointsPerDot = 10;
    static constexpr int kPointsPerPellet = 50;

    // Reset to a fresh game. Hi-score is preserved.
    void reset_for_new_game();

    int points() const {
        return m_points;
    }
    int hi_score() const {
        return m_hi_score;
    }
    int lives() const {
        return m_lives;
    }
    int level() const {
        return m_level;
    }

    void add_points(int pts);  // also bumps hi_score if exceeded
    void lose_life();          // -1 life; clamped at 0
    void add_life();           // +1 life; used by the extra-life-at-10000 reward
    bool game_over() const {
        return m_lives == 0;
    }

    void next_level();        // resets per-level counters; bumps level number
    void set_level(int lvl);  // explicit setter — used when loading a specific level by index

    // P11 will read/write hi-score from %APPDATA% via util::file. For now the
    // hi-score lives in memory only and resets when the process exits.
    void set_hi_score(int hi) {
        m_hi_score = hi;
    }

   private:
    int m_points = 0;
    int m_hi_score = 0;
    int m_lives = kStartingLives;
    int m_level = 1;
};

}  // namespace gameplay
