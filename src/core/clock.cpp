#include "core/clock.h"

#include <GL/freeglut.h>

#include <algorithm>

namespace core {

namespace {
// Cap a single frame's dt so a long stall (window dragged, debugger paused)
// doesn't trigger hundreds of catch-up update steps the next frame.
constexpr double kMaxFrameDt = 0.25;
} // namespace

Clock::Clock() {
    // Constructor may run during static init (before glutInit) so any
    // glutGet(GLUT_ELAPSED_TIME) here would be stale. Anchor real timing in
    // reset(), called from run_app after the GL context exists.
}

void Clock::reset() {
    m_prev_time = now_seconds();
    m_accumulator = 0.0;
    m_last_dt = 0.0;
    m_fps_window_start = m_prev_time;
    m_frames_in_window = 0;
    m_updates_in_window = 0;
}

int Clock::tick() {
    const double now = now_seconds();
    m_last_dt = now - m_prev_time;
    m_prev_time = now;

    m_accumulator += std::min(m_last_dt, kMaxFrameDt);

    int updates = 0;
    while (m_accumulator >= kFixedDt) {
        m_accumulator -= kFixedDt;
        ++updates;
    }

    m_updates_in_window += updates;
    return updates;
}

bool Clock::fps_ready() {
    ++m_frames_in_window;
    const double now = now_seconds();
    const double elapsed = now - m_fps_window_start;
    if (elapsed < 1.0) {
        return false;
    }
    m_last_fps = static_cast<int>(
        static_cast<double>(m_frames_in_window) / elapsed + 0.5);
    m_last_ups = static_cast<int>(
        static_cast<double>(m_updates_in_window) / elapsed + 0.5);
    m_frames_in_window = 0;
    m_updates_in_window = 0;
    m_fps_window_start = now;
    return true;
}

double Clock::now_seconds() const { return glutGet(GLUT_ELAPSED_TIME) * 0.001; }

} // namespace core
