#pragma once

namespace render {

// Window + tile geometry. The play area is a 28x31 grid of 24px tiles
// (672x744); the bottom 80px is reserved for the HUD.
inline constexpr int kWindowWidth = 672;
inline constexpr int kWindowHeight = 824;
inline constexpr int kPlayAreaWidth = 672;
inline constexpr int kPlayAreaHeight = 744;
inline constexpr int kHudHeight = 80;
inline constexpr int kTileSize = 24;
inline constexpr int kGridCols = 28;
inline constexpr int kGridRows = 31;

// One-time GL state setup (clear color, alpha blending, etc.).
// Call once after the window is created.
void init_gl();

// Re-establish viewport + ortho projection on every resize. Computes the
// largest centred sub-rect of the window that preserves the canonical
// 672:824 aspect ratio, so fullscreen and resized windows letterbox /
// pillarbox cleanly instead of stretching.
void on_reshape(int w, int h);

// Clear the entire framebuffer (including any letterbox bars) to black,
// then re-apply the letterbox viewport for subsequent draw calls. Call this
// at the start of every display frame.
void clear_and_apply_viewport();

}  // namespace render
