#include "core/app.h"

#include <GL/freeglut.h>

#include <cstdio>

#include "audio/audio.h"
#include "core/clock.h"
#include "core/game.h"
#include "input/input.h"
#include "render/gl_init.h"

namespace core {

namespace {

constexpr const char *kWindowTitle = "Pac-Man - freeglut - CSE 426";

// Module-local state. The GLUT callbacks are C-style function pointers, so we
// need a single anonymous-namespace instance for them to talk to.
Clock g_clock;
Game g_game;

bool g_is_fullscreen = false;
int g_windowed_w = render::kWindowWidth;
int g_windowed_h = render::kWindowHeight;

void toggle_fullscreen() {
    if (g_is_fullscreen) {
        glutReshapeWindow(g_windowed_w, g_windowed_h);
        glutPositionWindow(100, 60);
    } else {
        // glutFullScreen reports back via on_reshape — our letterbox math
        // there handles aspect-preservation automatically.
        glutFullScreen();
    }
    g_is_fullscreen = !g_is_fullscreen;
}

void on_display() {
    render::clear_and_apply_viewport();
    g_game.render();
    glutSwapBuffers();
}

// Wall-clock driven loop (~60 Hz). Using glutTimerFunc instead of
// glutIdleFunc because idle callbacks on Windows freeglut can stall when the
// window loses focus, which silently freezes the simulation.
constexpr unsigned int kFrameIntervalMs =
    16; // ~62.5 Hz; the accumulator absorbs the slop

void on_timer(int /*value*/) {
    if (input::state().quit_requested) {
        glutLeaveMainLoop();
        return;
    }

    // Fullscreen toggle is processed before Game::update consumes the press
    // flags — keeps the toggle responsive regardless of game state.
    if (input::state().press_fullscreen) {
        toggle_fullscreen();
    }

    const int updates = g_clock.tick();
    for (int i = 0; i < updates; ++i) {
        g_game.update(Clock::kFixedDt);
    }

    // Report FPS / UPS sparingly: every 5 s once gameplay starts.
    if (g_clock.fps_ready()) {
        static int s_seconds = 0;
        if ((++s_seconds % 5) == 0) {
            std::printf("[pacman] fps=%d  ups=%d\n", g_clock.last_fps(),
                        g_clock.last_ups());
        }
    }

    glutPostRedisplay();
    glutTimerFunc(kFrameIntervalMs, on_timer, 0);
}

void on_close() {
    // Window-X click. Mirrors Q so the main loop exits cleanly via the
    // single quit_requested gate in on_timer.
    input::state().quit_requested = true;
}

} // namespace

int run_app(int argc, char **argv) {
    // Disable stdout buffering — when this exe is launched without a console
    // (Win32 GUI subsystem, or stdout redirected to a pipe), the C runtime
    // block-buffers stdout and per-frame fflush isn't always honored. Going
    // unbuffered makes log lines visible in real time.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(render::kWindowWidth, render::kWindowHeight);
    glutInitWindowPosition(100, 60);
    glutCreateWindow(kWindowTitle);

    render::init_gl();
    g_clock.reset();

    // Audio init is non-fatal: the game runs silent on backend failure.
    audio::init();

    if (!g_game.init()) {
        std::printf("[pacman:FATAL] game::init() failed — exiting.\n");
        audio::shutdown();
        return 1;
    }

    glutDisplayFunc(on_display);
    glutReshapeFunc(render::on_reshape);
    glutKeyboardFunc(input::on_keyboard_down);
    glutKeyboardUpFunc(input::on_keyboard_up);
    glutSpecialFunc(input::on_special_down);
    glutSpecialUpFunc(input::on_special_up);
    glutCloseFunc(on_close);
    glutTimerFunc(kFrameIntervalMs, on_timer, 0);

    // Return from glutMainLoop on window close instead of exit()-ing — lets
    // us shut down cleanly through main().
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
                  GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    std::printf("[pacman] window opened (%dx%d). target update rate: %d Hz.\n",
                render::kWindowWidth, render::kWindowHeight,
                Clock::kUpdatesPerSecond);
    std::fflush(stdout);

    glutMainLoop();

    // P11: persist hi-score + volume preferences before audio shuts down,
    // so a fresh launch picks them up.
    g_game.save_user_settings();
    audio::shutdown();
    std::printf("[pacman] clean shutdown.\n");
    std::fflush(stdout);
    return 0;
}

} // namespace core
