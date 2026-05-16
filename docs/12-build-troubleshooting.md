# 12 — Build & Run Troubleshooting

Common issues a fresh-clone user is likely to hit, and how to fix them.

## CMake doesn't find a C++ compiler

Symptom:

```text
-- The CXX compiler identification is unknown
CMake Error: Could not find a working compiler...
```

Cause: CMake on Windows defaults to the Visual Studio generator, which
fails if MSVC isn't installed.

Fix: pass the generator explicitly.

```bash
cmake -G "MinGW Makefiles" -S . -B build
```

If you want to be explicit about which `g++` to use:

```bash
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=C:/mingw64/bin/g++.exe -S . -B build
```

## `cannot find -lfreeglut` at link time

The bundled freeglut `.dll.a` import-lib is missing or unreadable.

Fix:

1. Confirm `thirdparty/freeglut/lib/libfreeglut.dll.a` exists.
2. If the file is missing, restore it from a fresh clone or copy it
   from `C:/freeglut/lib/x64/`.

## Window opens, then closes silently

The exe ran but couldn't find `freeglut.dll` next to it.

Fix:

1. Confirm `build/bin/libfreeglut.dll` exists.
2. If not, the POST_BUILD copy step misfired. Re-run the build:
   `cmake --build build` — it'll copy the DLL alongside `pacman.exe`.

If the file exists but the window still closes, run `pacman.exe` from a
terminal so you can see the stderr message.

## Black screen, no audio

Audio init failure is non-fatal — the rest of the game runs silent.
Check the console for:

```text
[pacman:WARN] miniaudio engine init failed — running silent.
```

This commonly happens over Remote Desktop or in WSL where the audio
device isn't reachable. Plays fine on a normal interactive Windows
session.

## "audio: 0/8 SFX loaded"

The miniaudio engine started but the in-memory WAV decoder rejected
the synthesized bytes. This shouldn't happen — if you see it, it's a
real bug. Check that
[src/audio/audio.cpp](../src/audio/audio.cpp)'s `Synth::to_wav()`
header bytes are correct.

## Hi-score doesn't persist

The hi-score lives in `%APPDATA%/pacman-freeglut/savedata.txt`. If save
fails, you'll see:

```text
[pacman:WARN] could not save user settings to ...
```

Check that `%APPDATA%` is set in your environment and the user has
write permission. The game still works fine without it; only
persistence is lost.

## `glutBitmapCharacter` crashes / shows nothing

We use freeglut's built-in bitmap fonts. If they're not rendering at
all, the freeglut version is mismatched. The bundled freeglut v3.8.0
includes the bitmap fonts; check that
`thirdparty/freeglut/lib/x64/libfreeglut.dll.a` is the version that
ships with this repo, not a manually-installed one.

## Compiler complains about `-Wconversion` inside `miniaudio.h`

Should not happen — [src/audio/audio.cpp](../src/audio/audio.cpp)
wraps the include in a `#pragma GCC diagnostic` block that silences
`-Wconversion`, `-Wshadow`, `-Wpedantic`, and `-Wcast-function-type`
for that single TU. If you see warnings from miniaudio's lines, the
pragmas didn't take — check the file's first few lines.

## I want a clean rebuild

```bash
rm -rf build
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build -j
```

The first build takes ~30s because miniaudio.h is ~96 k lines.
Subsequent builds touching only our code are fast (~3s).

## Window flickers or letterbox bars are wrong

The reshape callback in
[src/render/gl_init.cpp](../src/render/gl_init.cpp) computes the
aspect-preserving viewport. If it's misbehaving, log
`g_full_window_w` / `g_viewport_w` to confirm the math. Most likely
cause: the windowing system reported a `(0, 0)` size during a minimize
event — there's a guard for that, but DPI scaling on a high-density
display might surface other edge cases.

## Game "feels slow" or framerate dips

Check the console for FPS / UPS lines (one every 5 s). If `ups=60` is
holding but `fps` is much lower, your GPU isn't keeping up — unusual
for a fixed-function GL game at 672 × 824, but possible on integrated
graphics with the window unfocused (Windows aggressively throttles
unfocused windows).

If `ups` itself is below 60, that's a real perf problem — investigate
[src/core/clock.cpp](../src/core/clock.cpp) for accumulator math or
profile `Game::update`.
