#pragma once

#include "render/primitives.h"

namespace render {
namespace particles {

// Single global particle pool. Emit-and-forget API: callers don't manage
// individual particle lifetimes — the pool reaps dead ones each tick.
// Game::update calls particles::update(dt) once per frame; Game::render
// calls particles::render() at the appropriate layer.
//
// The pool is sized to handle a few hundred particles without resizing —
// a death dissolve (~30 particles) plus a couple of dot-eat sparkles
// (~4 each) plus the ambient combo-popup activity stays well under the
// reserved cap.

void emit_sparkle(float x, float y, Color color);  // 4 small particles, ~200ms
void emit_burst(float x, float y, int count,
                Color color);                   // radial burst, ~500ms
void emit_ring(float x, float y, Color color);  // expanding ring (perk pickup)
void emit_dissolve(float x, float y,
                   Color color);  // ~30 particles, slow drift (death)

void update(float dt);
void render();
void clear();  // wipe everything — used on level reload

}  // namespace particles
}  // namespace render
