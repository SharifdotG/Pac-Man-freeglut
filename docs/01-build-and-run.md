# 01 — Build & Run

A complete clone-to-play walkthrough on Windows 11.

## 1. Prerequisites

Install these once. The project bundles everything else.

| Tool | Tested version | Where |
| --- | --- | --- |
| **MinGW-w64 g++** | 15.2.0 | `C:/mingw64/` (or any path on `PATH`) |
| **CMake** | 4.1.0 (≥ 3.20 required) | on `PATH` |
| **Git** | any | for cloning |

Verify they're on your `PATH`:

```bash
g++ --version   # should print g++ (MinGW-W64 ...) 13.x or newer
cmake --version # should print cmake version 3.20+
```

If `g++` isn't found, add `C:/mingw64/bin` to your `PATH`.

## 2. Clone and configure

```bash
git clone https://github.com/SharifdotG/Pac-Man-freeglut.git
cd pacman-freeglut
cmake -G "MinGW Makefiles" -S . -B build
```

**Why `-G "MinGW Makefiles"` is required:** on Windows, CMake defaults to MSVC's Visual Studio generator, which can't use g++. Always pass the generator explicitly.

You should see CMake report:

```text
-- The CXX compiler identification is GNU 15.2.0
-- ...
-- pacman_fg configured. Build with:  cmake --build .../build -j
-- Generating done
-- Build files have been written to: .../build
```

If CMake fails to find g++, either set `CMAKE_CXX_COMPILER` explicitly:

```bash
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=C:/mingw64/bin/g++.exe -S . -B build
```

…or fix your `PATH`.

## 3. Build

```bash
cmake --build build -j
```

Output:

```text
[ ## %] Building CXX object .../main.cpp.obj
...
[100%] Linking CXX executable bin/pacman.exe
[100%] Built target pacman
```

The post-build step copies `freeglut.dll` and `assets/` next to the exe automatically.

## 4. Run

```bash
./build/bin/pacman.exe
```

You can also double-click `pacman.exe` from File Explorer at `build/bin/`.

### What you should see (current build, P1)

A 672 × 824 dark-navy window titled "Pac-Man — CSE 426 (P1)". Console prints
the startup banner and one FPS / UPS sample per second:

```text
[pacman] window opened (672x824). target update rate: 60 Hz.
[pacman] fps=61  ups=60
[pacman] fps=61  ups=60
…
[pacman] clean shutdown.
```

`ups=60` is the simulation update rate (fixed step). `fps` is the render rate;
it follows your display vsync when the window has focus, and may dip to ~40
when the window is unfocused. Press **ESC**, **Q**, or close the window to
exit.

## 5. Clean rebuild

```bash
rm -rf build
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build -j
```

## 6. Common issues

See [12-build-troubleshooting.md](12-build-troubleshooting.md) (filled in as we hit issues).

Quick hits:

- **`cannot find -lfreeglut`** — `libfreeglut.dll.a` is missing from `thirdparty/freeglut/lib/`. Re-clone or restore from `C:/freeglut/lib/x64/`.
- **Window opens then closes immediately** — usually `freeglut.dll` isn't next to the exe. Re-run the build (the POST_BUILD step copies it) or manually copy from `thirdparty/freeglut/bin/`.
- **Black screen, no console output** — make sure you ran from a terminal, not File Explorer; otherwise stdout is silently discarded.
