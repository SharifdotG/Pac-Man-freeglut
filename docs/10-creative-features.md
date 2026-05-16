# 10 — Creative Features (Bonus)

The four bundles delivered for the 15 bonus marks, plus the supporting
machinery they share.

| Bundle | Modules | Status |
| --- | --- | --- |
| Unique ghost abilities | [game.cpp](../src/core/game.cpp) (Blinky/Clyde) | ✅ |
| Power-up perks | [perks.h/cpp](../src/gameplay/perks.h) | ✅ |
| Particles + screen shake | [particles.h/cpp](../src/render/particles.h), [camera.h/cpp](../src/render/camera.h) | ✅ |
| Combo system + popups | [combo.h/cpp](../src/gameplay/combo.h) | ✅ |

## 1. Unique ghost abilities

Pinky / Inky / Clyde already differ from Blinky via their canonical
chase targets (see [06-ghost-ai.md](06-ghost-ai.md)). On top of that,
**Blinky** and **Clyde** get one extra ability each.

### Blinky's fire-burst

A 3-state machine, ticking only while Blinky is in Chase / Scatter (no
firing while Frightened, Eaten, or InHouse).

| State | Duration | Visual | Hazard? |
| --- | --- | --- | --- |
| `Cooldown` | 12 s | normal Blinky | no |
| `Telegraph` | 0.5 s | translucent red halo around him | no |
| `Firing` | 0.4 s | 3 fire tiles in front of his snapshot origin | **yes** — Pac dies on overlap |

Implementation: `Game::update_blinky_ability` /
`Game::pac_in_blinky_fire` /  `Game::render_blinky_fire`. The fire
direction is captured at the moment firing begins (so Blinky can swerve
at the last second), and the burst is clipped at the first wall — fire
doesn't pass through the maze.

When the fire connects, the regular `on_pac_caught` path runs: life
lost, dissolve particles, screen shake, death beat.

### Clyde's clone

Every 30 s while Clyde is on the board, he spawns a translucent orange
**clone** at his current position. The clone:

- has the same speed as Clyde
- moves with **frightened-style random AI** (uniform random walkable
  direction at every tile boundary)
- lives 4 s, fading out as it ages
- causes a normal death-on-touch

Implementation: `Game::ClydeClone` struct + `update_clyde_clone` +
`render_clyde_clone` + `clone_overlaps_pac`. The clone reuses the
`(col, row, offset)` movement model used by ghosts but doesn't go
through the full Ghost / AI pipeline — it's deliberately lightweight.

## 2. Power-up perks

A separate pickup that briefly appears on a random walkable tile every
**25–40 s** and lives for **10 s** before despawning.

| Kind | Effect | Duration | Tint | HUD icon |
| --- | --- | --- | --- | --- |
| **Freeze** | All non-Frightened/Eaten/InHouse ghosts pause | 4 s | ice blue | snowflake |
| **Speed** | Pac speed × 1.40 | 5 s | gold | chevron » |
| **Invisibility** | Ghosts switch from Chase → Scatter (stop targeting Pac) | 4 s | pale grey | crossed eye |

Implementation: [src/gameplay/perks.h](../src/gameplay/perks.h) for
data + colours + the procedural icon, and `Game::update_perk_on_map` /
`Game::update_active_perk` / `Game::try_pickup_perk` /
`on_perk_activated` / `on_perk_expired` for the lifecycle.

Only one effect can be active at a time. Picking up a perk while
another is active replaces the current effect.

The HUD shows the active perk as a coloured icon in the bottom-right
along with a horizontal **timer bar** that drains as the effect winds
down. The play area also gets a subtle **screen tint** in the perk's
colour for the duration so the player has unmissable feedback that
something special is happening.

### Picking the spawn tile

`Game::pick_random_perk_tile` rejects:

- non-Empty tiles (walls, dots, pellets)
- the rectangular envelope around the ghost house (cols 10–17, rows
  13–17) — those Empty cells aren't reachable by Pac

It tries up to 32 random tiles before giving up; on failure the
spawn cooldown is set to 5 s and we try again next round. In practice
the maze has ~50 walkable Empty cells (the corridors get carved out as
Pac eats), so it almost always succeeds on the first attempt.

## 3. Particles + screen shake

[src/render/particles.cpp](../src/render/particles.cpp) — a single
global particle pool (~256 reserved), emit-and-forget API. Each
particle has position, velocity, lifetime, colour, size, and an
optional gravity term.

| Trigger | Function | Particles |
| --- | --- | --- |
| Dot eaten | `emit_sparkle` | 4 small particles, ~200 ms, no gravity |
| Frightened ghost eaten | `emit_burst` (count 14) | radial burst, 400-650 ms, gravity 60 |
| Fruit eaten | `emit_burst` (count 12) | warm-orange burst |
| Perk picked up | `emit_ring` | 16 particles in a clean ring |
| Pac death | `emit_dissolve` | 30 particles, slight upward bias, gravity 30 |
| Extra-life reward | `emit_burst` (count 16) | bright-yellow burst |

Particle lifetime drives alpha: each particle fades linearly from full
alpha at spawn to 0 at expiry.

[src/render/camera.cpp](../src/render/camera.cpp) — screen shake.
Currently only triggered on Pac death (8 px magnitude, 300 ms duration,
linear attenuation). The offset is applied via `glTranslatef` *only to
the world layer* — HUD and menus stay still, so score and life icons
don't appear to vibrate during the death beat.

## 4. Combo system + floating popups

[src/gameplay/combo.h](../src/gameplay/combo.h) — Pac's eat-streak
multiplier. Eating a dot within `kComboWindow = 0.4 s` of the previous
eat increments the chain count; otherwise it resets to 1.

```text
Multiplier = min(1.0 + 0.1 × (chain − 1), 3.0)
```

So x1.0 at chain 1, x1.1 at 2, x1.2 at 3, ... up to **x3.0 at chain 21+**.
The streak resets on a power pellet (which has its own scoring) and on
death. Pellet/ghost/fruit don't participate.

Floating popups ("+10", "+22 ×1.4", "+200", "EXTRA LIFE!") drift up 32
pixels and fade over 600 ms. Implementation: [combo.cpp](../src/gameplay/combo.cpp)
keeps a small `std::vector<Popup>` with pos/text/lifetime/colour. The
popup is drawn as text via `render::draw_text_centered`, with the
alpha derived from `lifetime / kPopupLifetime`.

Combo popups only show once the chain reaches 3 — single-dot pickups
stay quiet, but a chain of 5+ produces a constant stream of `+12 ×1.4`
floating up off Pac.

## Tuning knobs

| Knob | Value | File |
| --- | --- | --- |
| Blinky fire cooldown | 12 s | game.cpp `kBlinkyFireCooldown` |
| Telegraph window | 0.5 s | `kBlinkyTelegraphSec` |
| Firing window | 0.4 s | `kBlinkyFiringSec` |
| Fire range | 3 tiles | `kBlinkyFireRange` |
| Clyde clone cooldown | 30 s | `kClydeCloneCooldown` |
| Clyde clone lifetime | 4 s | `kClydeCloneLifetime` |
| Perk spawn cooldown | 25–40 s (random) | perks.h `kPerkSpawnMin/Max` |
| Perk on-map lifetime | 10 s | `kPerkLifetimeOnMap` |
| Freeze duration | 4 s | `kFreezeDuration` |
| Speed duration | 5 s | `kSpeedDuration` |
| Speed multiplier | × 1.40 | `kSpeedMultiplier` |
| Invisibility duration | 4 s | `kInvisibilityDuration` |
| Combo window | 0.4 s | combo.h `kComboWindow` |
| Combo per-link | +0.1× | `kComboPerLink` |
| Combo max | × 3.0 | `kComboMax` |
| Camera shake (death) | 8 px / 300 ms | game.cpp `kCameraShakeMagPx / Duration` |
| Popup lifetime | 600 ms | combo.cpp `kPopupLifetime` |
| Popup drift | 32 px | `kPopupDriftPx` |
| Audio cues | reuses existing SFX bank — see [08-audio.md](08-audio.md) | various |

## What's NOT in P10 (deferred to P11 polish)

- Bigger creative features: multi-level theme art, achievements,
  per-level fruit/scoring tweaks
- Particle textures (current particles are flat coloured quads; could
  use an additive sparkle texture)
- Per-perk SFX (currently they all use `PowerPellet` for pickup)
- Persisting the eat-streak chain across pellet pickups (reset is
  intentional and matches the player's mental model, but a "best
  combo this run" stat in the HUD would be a nice touch)
