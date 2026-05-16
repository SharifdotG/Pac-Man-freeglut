---
name: add-bonus-feature
description: Implement a creative bonus feature (perk, ghost ability, particle effect, scoring rule, etc.) consistently with project conventions.
---

# add-bonus-feature

Use this skill when adding a creative feature beyond the base game — e.g.
a new perk, a ghost ability variant, a particle effect, a scoring twist.
Bonus features are graded on **visibility + polish**, not technical depth.

## The bar for "bonus-worthy"

A feature counts toward the bonus marks if all four are true:

1. **Visible to the grader within 90 seconds** of starting a fresh playthrough.
2. **Has a clear in-game cue** (icon, sound, screen tint, popup).
3. **Doesn't crash or soft-lock** under any input.
4. **Documented** in [docs/10-creative-features.md](../../docs/10-creative-features.md).

If any of these is shaky, polish the existing features instead of adding a
new one.

## Steps

1. **Pick the right module.** A new feature should belong to exactly one
   module:

   | Feature kind | Lives in |
   |---|---|
   | New perk / power-up | `src/gameplay/perks.cpp` |
   | New ghost ability | `src/gameplay/ghost_ai.cpp` (+ `ghost.cpp` for state) |
   | New visual effect | `src/render/particles.cpp` (+ maybe `camera.cpp`) |
   | New scoring rule | `src/gameplay/score.cpp` or `combo.cpp` |
   | New menu item / UI | `src/ui/menu.cpp` (or relevant screen) |

   If the feature spans multiple modules, prefer adding glue in
   [src/core/game.cpp](../../src/core/game.cpp) over scattering state.

2. **Design first, code second.** Add a short design note to
   [docs/10-creative-features.md](../../docs/10-creative-features.md):
   - What it does (1 paragraph).
   - Trigger condition.
   - Effect duration / cooldown.
   - Visual / audio cue.
   - Failure modes considered.

3. **Wire the state machine.** Bonus features rarely need a new top-level
   state — they're modifiers on `PLAYING`. If you do need one (e.g. a "fever
   mode"), add the enum value and transition rule in `core/state.cpp` and
   `core/game.cpp`. Never sprinkle state changes throughout gameplay code.

4. **Reuse before inventing:**
   - Particles → use `render::particles_emit(...)`.
   - SFX → use existing `SfxId` if applicable; only add a new one via the
     [add-sound](add-sound.md) skill.
   - Sprites → use existing atlas regions if applicable; only extend via the
     [add-sprite](add-sprite.md) skill.

5. **Build and run:**

   ```bash
   cmake --build build -j
   ./build/bin/pacman.exe
   ```

6. **Verify** with the [verify-phase](verify-phase.md) checklist + bonus
   criteria above. Watch a teammate (or imagine one) play for 90 s — does the
   feature show up? Is it obvious what's happening?

7. **Update docs:** complete the entry in
   [docs/10-creative-features.md](../../docs/10-creative-features.md) (move it
   from "planned" to "shipped"), add a screenshot to `docs/screenshots/`, and
   reference it from the README's Highlights bullet list.

## What NOT to do

- Don't add new third-party libraries. Use what's bundled.
- Don't add a feature that requires the grader to read a manual to discover.
  If it's not obvious in the first minute, it's not bonus-worthy.
- Don't break existing features for the sake of a new one. Bonus features are
  additive.
