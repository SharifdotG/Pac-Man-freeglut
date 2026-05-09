#pragma once

#include <optional>

#include "gameplay/perks.h"
#include "gameplay/score.h"

namespace ui {

// Render the bottom 80px HUD strip: score, hi-score, level number, life
// icons, elapsed game time (mm:ss), and (when set) the active-perk badge
// with its remaining-time progress bar. Called once per frame after the
// maze + actors so the HUD always sits on top.
//
// `play_time_seconds` is the elapsed *gameplay* time — paused / dying /
// menu time is not counted, matching the assignment's "win in the
// minimum time" objective.
void render_hud(const gameplay::Score& s,
                const std::optional<gameplay::ActivePerk>& active_perk,
                float play_time_seconds);

}  // namespace ui
