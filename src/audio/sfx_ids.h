#pragma once

namespace audio {

// Index into the audio module's pre-synthesized SFX bank. Order is
// arbitrary; the synthesis code maps each ID to a specific tone pattern.
enum class SfxId : int {
    Chomp = 0,    // dot pickup
    PowerPellet,  // power-pellet pickup
    EatGhost,     // chomp a frightened ghost
    EatFruit,     // bonus fruit
    Death,        // Pac caught
    ExtraLife,    // score crosses 10 000 (one-shot)
    LevelStart,   // jingle on level load
    MenuClick,    // menu navigation / confirm

    Count,  // sentinel
};

}  // namespace audio
