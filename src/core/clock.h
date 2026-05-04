#pragma once

namespace core {

// Fixed-timestep accumulator + rolling FPS / UPS counter.
//
// Drive `tick()` from the GLUT idle callback once per render iteration; it
// returns how many fixed-dt update steps the simulation should run this frame.
// Call `fps_ready()` afterwards to find out whether a one-second window just
// closed (and a fresh FPS sample is available via `last_fps()`).
class Clock {
  public:
    static constexpr int kUpdatesPerSecond = 60;
    static constexpr double kFixedDt = 1.0 / kUpdatesPerSecond;

    Clock();

    // Re-anchor the clock to "now" — call once after glutInit() so the
    // static-init Clock instance doesn't compute dt from time-since-process-
    // -start instead of time-since-glutInit.
    void reset();

    // Advance the wall clock and accumulate elapsed time. Returns the number
    // of fixed-dt steps the caller should run this frame (typically 0–2;
    // capped internally to avoid spiral-of-death after a long stall).
    int tick();

    // Last frame's wall-clock dt (seconds). Useful for non-physical
    // animation (e.g. fade interpolation) that doesn't need fixed-step.
    double dt_seconds() const { return m_last_dt; }

    // Returns true at most once per ~1s when a new FPS/UPS sample is ready.
    bool fps_ready();

    int last_fps() const { return m_last_fps; }
    int last_ups() const { return m_last_ups; }

  private:
    double now_seconds() const;

    double m_prev_time = 0.0;
    double m_accumulator = 0.0;
    double m_last_dt = 0.0;

    double m_fps_window_start = 0.0;
    int m_frames_in_window = 0;
    int m_updates_in_window = 0;
    int m_last_fps = 0;
    int m_last_ups = 0;
};

} // namespace core
