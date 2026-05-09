#pragma once

#include "gameplay/score.h"

namespace ui {

// Render the bottom 80px HUD strip: score, hi-score, level and remaining lives.
void render_hud(const gameplay::Score& s);

}  // namespace ui
