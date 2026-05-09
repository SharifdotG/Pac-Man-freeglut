#include "gameplay/modes.h"

namespace gameplay {

namespace {
// Phase durations in seconds. Indices 0,2,4,6 are Scatter; 1,3,5 are Chase;
// after phase 6 we sit in Chase forever.
constexpr float kPhaseDurations[7] = {7.0f, 20.0f, 7.0f, 20.0f, 5.0f, 20.0f, 5.0f};
constexpr int kPhaseCount = 7;

WaveMode mode_for_phase(int phase) {
    return (phase % 2 == 0) ? WaveMode::Scatter : WaveMode::Chase;
}
}  // namespace

void WaveTimer::reset() {
    m_phase = 0;
    m_phase_elapsed = 0.0f;
    m_mode = WaveMode::Scatter;
    m_just_changed = false;
}

void WaveTimer::update(float dt) {
    if (m_phase >= kPhaseCount) {
        // Locked in Chase forever.
        return;
    }
    m_phase_elapsed += dt;
    if (m_phase_elapsed >= kPhaseDurations[m_phase]) {
        m_phase_elapsed -= kPhaseDurations[m_phase];
        ++m_phase;
        const WaveMode new_mode =
            (m_phase >= kPhaseCount) ? WaveMode::Chase : mode_for_phase(m_phase);
        if (new_mode != m_mode) {
            m_mode = new_mode;
            m_just_changed = true;
        }
    }
}

bool WaveTimer::consume_just_changed() {
    const bool v = m_just_changed;
    m_just_changed = false;
    return v;
}

}  // namespace gameplay
