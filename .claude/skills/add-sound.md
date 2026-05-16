---
name: add-sound
description: Add a new sound effect or music track to the audio engine, hook it to a game event, and verify it plays.
---

# add-sound

Use this skill when you need to add a sound — a new SFX for a feature, an
alternate BGM track, etc. Audio is owned by [src/audio/audio.cpp](../../src/audio/audio.cpp)
which wraps [miniaudio](https://github.com/mackron/miniaudio).

## File format expectations

| Type | Format | Why |
|---|---|---|
| SFX | 16-bit PCM **WAV**, 22050 Hz, mono | small, fast load, no decode cost |
| BGM | **OGG Vorbis** | small file size for long loops |

If the source is in another format, convert with Audacity or ffmpeg:

```bash
ffmpeg -i input.mp3 -ar 22050 -ac 1 -c:a pcm_s16le output.wav
```

## Steps

1. **Drop the file** in `assets/audio/`. Naming convention: `sfx_<name>.wav`
   or `bgm_<name>.ogg`.

2. **Add an enum value** in [src/audio/sfx_ids.h](../../src/audio/sfx_ids.h):

   ```cpp
   enum class SfxId {
       // ... existing ...
       MyNewSound,
       _Count   // keep last
   };
   ```

3. **Register the file path** in `audio.cpp` — there's a `kSfxPaths` array
   (or similar) that maps `SfxId` to a relative path under `assets/audio/`.
   Add the corresponding entry.

4. **Trigger the sound** from the call site:

   ```cpp
   #include "audio/audio.h"
   // ...
   audio::play(audio::SfxId::MyNewSound);
   ```

5. **Build and run:**

   ```bash
   cmake --build build -j
   ./build/bin/pacman.exe
   ```

6. **Verify:**
   - The sound plays at the right moment.
   - No console warnings like "audio: failed to load …".
   - Volume is reasonable relative to other SFX (re-master in Audacity if not —
     don't try to normalise in-engine).

7. **Document:** add a row to the SFX table in
   [docs/08-audio.md](../../docs/08-audio.md) — name, source URL/credit, when
   it plays.

## Asset sourcing

For new sounds, pull from CC0 sources only (or self-record):

- [Freesound.org](https://freesound.org) — filter for "CC0" license
- [OpenGameArt.org](https://opengameart.org) — chiptune / arcade tagged
- [Sonniss GDC bundles](https://sonniss.com/gameaudiogdc) — annual free release

**Always credit** the original creator in `docs/04-asset-pipeline.md`, even
for CC0 (it's a courtesy and protects the project from accidental
mis-attribution).

## Common pitfalls

- **Path resolution:** the runtime looks for assets relative to
  `<exe-dir>/assets/`. The POST_BUILD step copies `assets/` automatically, but
  if you skip the build and just drop a file in the source tree, `pacman.exe`
  won't find it.
- **Sample-rate mismatch:** miniaudio resamples on the fly, but a 44.1 kHz
  stereo file is 4× the memory of our 22 kHz mono budget. Convert before
  committing.
- **Loop point clicks:** for BGM, ensure the loop point is on a zero-crossing
  in Audacity to avoid an audible click at wrap.
