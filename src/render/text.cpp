#include "render/text.h"

#include <GL/freeglut.h>

namespace render {

namespace {
// HELVETICA_18 reads cleanly at native size; the only freeglut fonts wider
// than this get blocky. Swap to a real pixel-art font sheet in P11 polish
// if it looks too generic.
void* const kFont = GLUT_BITMAP_HELVETICA_18;
}  // namespace

void draw_text(float x_left, float y_baseline, const char* str) {
    if (!str)
        return;
    glRasterPos2f(x_left, y_baseline);
    for (const char* p = str; *p; ++p) {
        glutBitmapCharacter(kFont, *p);
    }
}

int text_width_px(const char* str) {
    if (!str)
        return 0;
    int w = 0;
    for (const char* p = str; *p; ++p) {
        w += glutBitmapWidth(kFont, *p);
    }
    return w;
}

void draw_text_centered(float x_center, float y_baseline, const char* str) {
    const int w = text_width_px(str);
    draw_text(x_center - static_cast<float>(w) * 0.5f, y_baseline, str);
}

}  // namespace render
