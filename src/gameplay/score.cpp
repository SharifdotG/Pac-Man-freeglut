#include "gameplay/score.h"

namespace gameplay {

void Score::reset_for_new_game() {
    m_points = 0;
    m_lives = kStartingLives;
    m_level = 1;
    // hi_score is preserved across games on purpose.
}

void Score::add_points(int pts) {
    if (pts <= 0)
        return;
    m_points += pts;
    if (m_points > m_hi_score) {
        m_hi_score = m_points;
    }
}

void Score::lose_life() {
    if (m_lives > 0) {
        --m_lives;
    }
}

void Score::add_life() {
    ++m_lives;
}

void Score::next_level() {
    ++m_level;
}

void Score::set_level(int lvl) {
    if (lvl >= 1)
        m_level = lvl;
}

}  // namespace gameplay
