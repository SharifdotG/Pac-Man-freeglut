#pragma once

namespace core {

// Top-level game state. The transition table lives in core/game.cpp and the
// per-state render dispatch in Game::render(). Gameplay code never mutates
// this directly.
enum class GameState : unsigned char {
    Splash,        // 1.5s intro fade on boot — skippable
    Menu,          // main menu
    Help,          // controls + ghost legend
    Settings,      // audio volume sliders
    Ready,         // P12: brief "READY!" beat before play resumes
    Playing,       // active play
    Paused,        // overlay over Playing; sim halted, render continues
    Dying,         // Pac just got caught — actor updates frozen for a beat
    LevelComplete, // all dots eaten — brief celebration
    GameOver,      // lives = 0 — game-over screen with Retry / Menu
    WinScreen,     // final level cleared — you-win screen with Restart / Menu
};

inline const char *state_name(GameState s) {
    switch (s) {
    case GameState::Splash:
        return "Splash";
    case GameState::Menu:
        return "Menu";
    case GameState::Help:
        return "Help";
    case GameState::Settings:
        return "Settings";
    case GameState::Ready:
        return "Ready";
    case GameState::Playing:
        return "Playing";
    case GameState::Paused:
        return "Paused";
    case GameState::Dying:
        return "Dying";
    case GameState::LevelComplete:
        return "LevelComplete";
    case GameState::GameOver:
        return "GameOver";
    case GameState::WinScreen:
        return "WinScreen";
    }
    return "?";
}

} // namespace core
