#include "render/gl_init.h"

#include <GL/freeglut.h>

namespace render {

namespace {

// Tracked across reshape so display() can clear the *full* framebuffer (so
// letterbox bars are black) before restoring the centred letterbox viewport
// for the actual scene draw.
int g_full_window_w = kWindowWidth;
int g_full_window_h = kWindowHeight;

int g_viewport_x = 0;
int g_viewport_y = 0;
int g_viewport_w = kWindowWidth;
int g_viewport_h = kWindowHeight;

void apply_logical_projection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<double>(kWindowWidth), static_cast<double>(kWindowHeight), 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

}  // namespace

void init_gl() {
    // Black so letterbox bars in fullscreen are invisible against the
    // play-area background.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void on_reshape(int w, int h) {
    if (w <= 0 || h <= 0)
        return;
    g_full_window_w = w;
    g_full_window_h = h;

    // Pick the largest centred sub-rect that matches our 672:824 aspect.
    const float target_aspect =
        static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight);
    const float window_aspect = static_cast<float>(w) / static_cast<float>(h);

    if (window_aspect > target_aspect) {
        // Window is wider than target → pillarbox (vertical bars).
        g_viewport_h = h;
        g_viewport_w = static_cast<int>(static_cast<float>(h) * target_aspect);
        g_viewport_x = (w - g_viewport_w) / 2;
        g_viewport_y = 0;
    } else {
        // Window is taller (or same) → letterbox (horizontal bars).
        g_viewport_w = w;
        g_viewport_h = static_cast<int>(static_cast<float>(w) / target_aspect);
        g_viewport_x = 0;
        g_viewport_y = (h - g_viewport_h) / 2;
    }

    glViewport(g_viewport_x, g_viewport_y, g_viewport_w, g_viewport_h);
    apply_logical_projection();
}

void clear_and_apply_viewport() {
    // Step 1: clear the entire framebuffer (so the letterbox bars are
    // properly black even when the previous frame's viewport was smaller).
    glViewport(0, 0, g_full_window_w, g_full_window_h);
    glClear(GL_COLOR_BUFFER_BIT);

    // Step 2: restore the aspect-preserving viewport for scene drawing.
    glViewport(g_viewport_x, g_viewport_y, g_viewport_w, g_viewport_h);
    apply_logical_projection();
}

}  // namespace render
