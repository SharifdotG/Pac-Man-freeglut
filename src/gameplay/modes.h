#pragma once

namespace gameplay {

// The single global mode that drives all non-Frightened, non-Eaten ghosts.
enum class WaveMode : unsigned char {
    Scatter,  // ghosts head for their personal corner
    Chase,    // ghosts hunt Pac via personality-specific target tiles
};

// Classic Pac-Man scatter/chase schedule. Time-based (level-1 values from
// the arcade); P7 will optionally swap in level-scaled tables.
//
//   phase 0: 7s Scatter
//   phase 1: 20s Chase
//   phase 2: 7s Scatter
//   phase 3: 20s Chase
//   phase 4: 5s Scatter
//   phase 5: 20s Chase
//   phase 6: 5s Scatter
//   phase 7+: Chase forever
//
// `consume_just_changed()` reports the single frame when the mode flipped
// so the game loop can flag every active ghost for an immediate
// direction-reversal — that reversal is the canonical visual signal that
// the wave changed.
class WaveTimer {
   public:
    void reset();
    void update(float dt);

    WaveMode current_mode() const {
        return m_mode;
    }

    // True exactly once per mode change. Calling it clears the flag.
    bool consume_just_changed();

   private:
    int m_phase = 0;
    float m_phase_elapsed = 0.0f;
    WaveMode m_mode = WaveMode::Scatter;
    bool m_just_changed = false;
};

}  // namespace gameplay
