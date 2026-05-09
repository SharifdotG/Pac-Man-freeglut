#pragma once

#include "util/direction.h"

namespace input {

// Per-frame input snapshot read by Game.
//
// `wanted` is sticky (Pac's direction buffer — survives across frames until
// applied or overridden). The `press_*` flags are EDGE-TRIGGERED: set on the
// frame a key is first pressed, cleared by `clear_press_flags()` after Game
// has consumed them. This makes them work correctly for menu navigation.
//
// `quit_requested` is the hard-quit gate: window-X click or `Q` key.
// `press_back` (Esc) is a soft "go back one screen" signal that Game routes
// through the state machine — only Menu turns it into quit_requested.
struct State {
    bool quit_requested = false;
    util::Direction wanted = util::Direction::None;

    bool press_enter = false;
    bool press_back = false;        // Esc — "back one level"
    bool press_pause = false;       // P
    bool press_fullscreen = false;  // F11
    bool press_up = false;
    bool press_down = false;
    bool press_left = false;
    bool press_right = false;
};

State& state();

// Called by Game at the END of update() — clears every press_* flag so they
// behave as one-shot edge events. wanted/quit_requested are NOT cleared.
void clear_press_flags();

void on_keyboard_down(unsigned char key, int x, int y);
void on_keyboard_up(unsigned char key, int x, int y);
void on_special_down(int key, int x, int y);
void on_special_up(int key, int x, int y);

}  // namespace input
