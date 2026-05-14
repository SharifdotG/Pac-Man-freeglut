#pragma once

#include "render/primitives.h"

namespace gameplay {

enum class PerkKind : unsigned char {
    Freeze,        // pauses all hostile ghosts
    Speed,         // boosts Pac speed
    Invisibility,  // ghosts ignore Pac (head to scatter corners)
};

// Perk pickup currently sitting on the maze. `active=false` means there
// isn't one — the field reuses the same trick as `Fruit`.
struct Perk {
    bool active = false;
    int col = 0;
    int row = 0;
    PerkKind kind = PerkKind::Freeze;
    float lifetime_remaining = 0.0f;  // counts down on the map; 0 → despawn
};

// Active effect on Pac (or the ghosts) AFTER a perk is picked up.
// Engine-style timer + kind. Only one active effect at a time; eating a
// new perk overrides the current effect.
struct ActivePerk {
    PerkKind kind = PerkKind::Freeze;
    float remaining = 0.0f;
};

// Tunables — kept here so docs/skills can reference them without digging.
constexpr float kPerkSpawnMin = 25.0f;
constexpr float kPerkSpawnMax = 40.0f;
constexpr float kPerkLifetimeOnMap = 10.0f;

constexpr float kFreezeDuration = 4.0f;
constexpr float kSpeedDuration = 5.0f;
constexpr float kInvisibilityDuration = 4.0f;
constexpr float kSpeedMultiplier = 1.40f;  // Pac speed × 1.40 while Speed active

const char* perk_short_name(PerkKind k);
render::Color perk_color(PerkKind k);
render::Color perk_screen_tint(PerkKind k);
float perk_duration(PerkKind k);

// Procedural icon used both on the maze (waiting to be picked up) and on
// the HUD as the active-effect badge. Drawn at (cx, cy) with `radius` px.
void perk_draw_icon(float cx, float cy, float radius, PerkKind k);

}  // namespace gameplay
