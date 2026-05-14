#pragma once

namespace ui {

enum class PauseItem : int {
    Resume = 0,
    Menu = 1,
    Count = 2,
};

// Render a translucent darkening overlay + the pause menu on top of the
// (already-rendered) play scene. Caller draws the gameplay first, then
// invokes this — render order matters.
void render_pause_overlay(int selected);

}  // namespace ui
