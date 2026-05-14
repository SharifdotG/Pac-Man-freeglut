#pragma once

namespace ui {

// Main menu items. Order matters — the integer index drives selection
// (Up/Down) and Game::handle_menu_select() dispatches by value.
enum class MainMenuItem : int {
    Start = 0,
    Help = 1,
    Settings = 2,
    Exit = 3,
    Count = 4,
};

// Render the main menu. `selected` is the currently-highlighted item.
// `hi_score` is shown in the corner to give players a long-term goal.
void render_main_menu(int selected, int hi_score);

}  // namespace ui
