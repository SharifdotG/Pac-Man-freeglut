#pragma once

namespace render {

// Bitmap text rendering on top of freeglut's built-in fonts. We use the
// HELVETICA_18 glyph set because it's monospaced enough for HUD numerals
// and large enough to read at our window size without scaling tricks.
//
// Position semantics: `x_left` is the leftmost pixel; `y_baseline` is the
// world-y of the text baseline (descenders sit a few pixels below). The
// current `glColor*` controls colour.
//
// Why this works under our y-down ortho: glBitmap is a raster operation —
// the bitmap's "up" is fixed in framebuffer space, not affected by the
// projection. With ortho `gluOrtho2D(0, w, h, 0)` and viewport y=0 at the
// framebuffer bottom, a bitmap drawn at world-y=Y appears with its
// baseline at world-y=Y, extending upward (smaller y) to its glyph height.
// Right-side-up.
void draw_text(float x_left, float y_baseline, const char* str);

void draw_text_centered(float x_center, float y_baseline, const char* str);

// Pixel width of `str` in the current font. Useful for right-aligning.
int text_width_px(const char* str);

}  // namespace render
