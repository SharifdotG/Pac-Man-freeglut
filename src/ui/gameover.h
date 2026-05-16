#pragma once

namespace ui {

// Both Game-Over and Win-Screen share the same pair of menu items, just
// with different titles and colours.
enum class GameOverItem : int {
    Retry = 0, // restart from level 1, fresh score
    Menu = 1,  // back to main menu
    Count = 2,
};

void render_game_over(int selected, int final_score, float play_time_seconds);
void render_win_screen(int selected, int final_score, float play_time_seconds);

} // namespace ui
