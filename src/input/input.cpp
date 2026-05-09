#include "input/input.h"

#include <GL/freeglut.h>

namespace input {

namespace {
State g_state;
}  // namespace

State& state() {
    return g_state;
}

void clear_press_flags() {
    g_state.press_enter = false;
    g_state.press_back = false;
    g_state.press_pause = false;
    g_state.press_fullscreen = false;
    g_state.press_up = false;
    g_state.press_down = false;
    g_state.press_left = false;
    g_state.press_right = false;
}

void on_keyboard_down(unsigned char key, int /*x*/, int /*y*/) {
    constexpr unsigned char kEsc = 27;
    constexpr unsigned char kEnter = 13;

    // Hard-quit only on Q. Esc is "back" — routed through Game.
    if (key == 'q' || key == 'Q') {
        g_state.quit_requested = true;
        return;
    }
    if (key == kEsc) {
        g_state.press_back = true;
        return;
    }
    if (key == kEnter) {
        g_state.press_enter = true;
        return;
    }
    if (key == 'p' || key == 'P') {
        g_state.press_pause = true;
        return;
    }

    switch (key) {
        case 'w':
        case 'W':
            g_state.wanted = util::Direction::Up;
            g_state.press_up = true;
            break;
        case 'a':
        case 'A':
            g_state.wanted = util::Direction::Left;
            g_state.press_left = true;
            break;
        case 's':
        case 'S':
            g_state.wanted = util::Direction::Down;
            g_state.press_down = true;
            break;
        case 'd':
        case 'D':
            g_state.wanted = util::Direction::Right;
            g_state.press_right = true;
            break;
        default:
            break;
    }
}

void on_keyboard_up(unsigned char /*key*/, int /*x*/, int /*y*/) {
    // Don't clear `wanted` — buffer persists. Don't clear press_* either —
    // that's the responsibility of clear_press_flags() at end-of-frame.
}

void on_special_down(int key, int /*x*/, int /*y*/) {
    switch (key) {
        case GLUT_KEY_UP:
            g_state.wanted = util::Direction::Up;
            g_state.press_up = true;
            break;
        case GLUT_KEY_DOWN:
            g_state.wanted = util::Direction::Down;
            g_state.press_down = true;
            break;
        case GLUT_KEY_LEFT:
            g_state.wanted = util::Direction::Left;
            g_state.press_left = true;
            break;
        case GLUT_KEY_RIGHT:
            g_state.wanted = util::Direction::Right;
            g_state.press_right = true;
            break;
        case GLUT_KEY_F11:
            g_state.press_fullscreen = true;
            break;
        default:
            break;
    }
}

void on_special_up(int /*key*/, int /*x*/, int /*y*/) {}

}  // namespace input
