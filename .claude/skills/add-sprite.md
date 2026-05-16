---
name: add-sprite
description: Add a new sprite or animation frame to the texture atlas, expose it by name, and verify it draws in-game.
---

# add-sprite

Use this skill when you need to draw something new on screen — a new ghost
ability effect, a perk icon, a UI element, etc. The game uses a single atlas
(`assets/sprites/atlas.png`) with named UV rects in `assets/sprites/atlas.json`.

## Why a single atlas?

Fewer texture binds → simpler renderer → no z-fighting between layers.
Adding a sprite means **editing the atlas**, not committing a new PNG.

## Steps

1. **Pick a free 24×24 cell** in `atlas.png`. Open the atlas (e.g. in GIMP /
   Krita / Aseprite). Naming convention for cells: see
   [docs/04-asset-pipeline.md](../../docs/04-asset-pipeline.md). Existing
   regions are mapped by `atlas.json`.

2. **Draw the sprite** at 24×24 pixels (or N × 24 for an animation strip).
   Save the modified PNG over `assets/sprites/atlas.png`.

3. **Add a named entry to `atlas.json`** with the pixel rect:

   ```json
   {
     "name": "perk_invis",
     "x": 240, "y": 264, "w": 24, "h": 24
   }
   ```

   For animation strips, add one entry per frame with a numeric suffix:
   `"perk_invis_0"`, `"perk_invis_1"`, etc.

4. **Reference the sprite in code.** Use `render::draw_sprite("name", x, y)`
   from wherever you're drawing. `atlas.cpp` resolves the name to UV coords at
   runtime (with a hash-map cache).

5. **Build and run:**

   ```bash
   cmake --build build -j
   ./build/bin/pacman.exe
   ```

6. **Verify:**
   - Sprite appears at the right place / time.
   - No visible seams or wrong-tile bleed (atlas filtering is `GL_NEAREST`
     so you should see clean pixel boundaries).
   - Console has no "atlas: unknown sprite name" warnings.

7. **Document:** if you added a new region (not just a new frame in an existing
   strip), update the atlas-layout table in
   [docs/04-asset-pipeline.md](../../docs/04-asset-pipeline.md).

## Common pitfalls

- **Off-by-one rect:** `(x, y, w, h)` is in pixels with top-left origin. A 24×24
  sprite at the top-left corner has rect `(0, 0, 24, 24)`, not `(1, 1, 24, 24)`.
- **Forgetting to save the JSON:** the PNG change is invisible if `atlas.json`
  doesn't expose a name for it.
- **Premultiplied vs straight alpha:** atlas is straight alpha. If a sprite has
  a halo around its edges, your image editor exported with the wrong setting.
