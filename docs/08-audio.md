# 08 — Audio

How the game makes noise. Two unusual choices to flag up front:

1. **No audio asset files are shipped.** Every SFX and the BGM are
   procedurally synthesized in memory at startup as 22050 Hz 16-bit PCM
   WAV byte arrays, then handed to miniaudio. Trade-off: the sounds are
   chiptune-style rather than recorded — appropriate for an arcade
   clone, zero licensing risk, and the build stays self-contained.
2. **`-Wconversion` and `-Wshadow` are suppressed in the audio TU only.**
   miniaudio's internals trip those flags hundreds of times. The
   `#pragma GCC diagnostic` block in [audio.cpp](../src/audio/audio.cpp)
   wraps just the `#include <miniaudio.h>` line.

Authoritative code:
[src/audio/audio.cpp](../src/audio/audio.cpp),
[src/audio/audio.h](../src/audio/audio.h),
[src/audio/sfx_ids.h](../src/audio/sfx_ids.h).

## Engine

`audio::init()` boots a single `ma_engine` with default config (lets
miniaudio pick the best Windows backend — usually WASAPI). Init failure
is **non-fatal** — `play()` / `start_bgm()` / volume APIs all become
no-ops, the rest of the game runs normally without sound. This matters
for headless / WSL / RDP scenarios where audio devices aren't reachable.

`audio::shutdown()` is called from `core/app.cpp` after `glutMainLoop`
returns, before the process exits.

## SFX bank

Eight one-shot sounds, all pre-decoded into `ma_sound` slots at startup:

| ID | Trigger | Patch |
| --- | --- | --- |
| `Chomp` | Pac eats a small dot | 60 ms 560 Hz square, fade |
| `PowerPellet` | Pac eats a power pellet | 4-note trill 540↑820↓540↑820 |
| `EatGhost` | Pac eats a frightened ghost | 300 ms sweep 220→880 Hz |
| `EatFruit` | Pac eats the bonus fruit | 2-note arpeggio 880, 1320 Hz |
| `Death` | Pac caught by ghost | 1.2 s descending sweep 880→110 Hz |
| `ExtraLife` | Score crosses 10 000 (once per game) | C5/E5/G5 major triad |
| `LevelStart` | Level loaded | C5/E5/G5/C6 ascending |
| `MenuClick` | Menu navigation / confirm | 30 ms 440 Hz square blip |

Each is created by a `Synth` helper that supports `add_sine`,
`add_square`, `add_sweep`, `add_silence`, and `add_wobble_square`. All
amplitudes are conservatively below 0.5 to avoid clipping when multiple
sounds overlap.

## BGM

Single 4-second looping square wave with frequency modulation —
**center 140 Hz, ±40 Hz wobble at 2 Hz**. Not a melody; it's the same
"siren" pattern the original arcade used as gameplay backing track.

`ma_sound_set_looping(MA_TRUE)` ensures seamless loop. miniaudio handles
the join automatically when looping a fully-decoded sound.

`audio::start_bgm()` / `stop_bgm()` are idempotent — safe to call from
any state. `bgm_is_playing()` returns the current flag.

The Game state machine starts BGM on `load_level()` and stops it on:

- death (during `Dying`)
- pause (during `Paused`)
- level complete (banner is silent)
- transition to Menu / GameOver / WinScreen

## Volume controls

Three independent volumes, all in `[0, 1]`, all stored in module-static
floats:

| Control | Default | Affects |
| --- | --- | --- |
| Master | 0.70 | All SFX + BGM (multiplicative) |
| SFX | 0.80 | Per-SFX volume |
| BGM | 0.40 | Background music only |

Effective per-sound volume is `master × {sfx,bgm}`. The Settings screen
(`GameState::Settings`) exposes all three as horizontal sliders adjusted
in 5% steps with Left / Right keys. Up / Down navigates between the
three rows.

The SFX slider plays a `MenuClick` on every left/right press to give
audible feedback at the new volume.

## In-memory WAV format

Each synthesized sound is a 16-bit mono PCM WAV with a minimal RIFF
header:

```text
[44-byte header][N * 2 bytes of int16 samples]
```

Header layout (little-endian):

| Offset | Size | Field |
| --- | --- | --- |
| 0 | 4 | `RIFF` |
| 4 | 4 | total file size − 8 |
| 8 | 4 | `WAVE` |
| 12 | 4 | `fmt` |
| 16 | 4 | fmt chunk size = 16 |
| 20 | 2 | PCM format = 1 |
| 22 | 2 | channels = 1 |
| 24 | 4 | sample rate = 22050 |
| 28 | 4 | byte rate = 44100 |
| 32 | 2 | block align = 2 |
| 34 | 2 | bits per sample = 16 |
| 36 | 4 | `data` |
| 40 | 4 | data size in bytes |
| 44 | … | samples |

`ma_decoder_init_memory()` parses this byte buffer; we then init a
`ma_sound` from the decoder with `MA_SOUND_FLAG_DECODE` so miniaudio
fully decodes upfront. The byte buffer is kept alive in `SoundSlot` for
the lifetime of the engine (decoders read directly from it).

## Adding a new SFX

Use the [add-sound skill](../.claude/skills/add-sound.md) — but note
that path no longer requires a WAV file. Instead:

1. Add a new value to `audio::SfxId` enum in
   [sfx_ids.h](../src/audio/sfx_ids.h) (before `Count`).
2. Add a `case` to `make_sfx()` in
   [audio.cpp](../src/audio/audio.cpp) describing the patch with the
   `Synth` helper (`add_sine`, `add_square`, `add_sweep`, …).
3. Trigger from the call site: `audio::play(audio::SfxId::Whatever)`.
4. Add a row to the table above.

No file copy / asset pipeline / path resolution needed.

## What's NOT here (deferred to P11 polish)

- Recorded WAV / OGG assets replacing the synth (warmer sound)
- Per-state BGM tracks (menu music, frightened-mode music, etc.)
- Audio ducking (lower BGM during important SFX)
- Persistent volume preferences (currently reset every launch)
