# Plan for Daily Commits — CSE 426 Pac-Man

> **Authors of the project:** Sharif, Priom, Ovi
> **Window for "fake-but-realistic" daily history:** May 2 → May 16, 2026 (15 days, 17 commits total)
> **Final state on May 16:** repo at version **v1.0.0**, identical in content to the finished local copy of the game.

This document is the single source of truth for *how* the three of you will publish the already-finished game to a separate Git repository as if it were being built up day-by-day. Read **Section 0 → Section 4** in order before doing anything. After that, every day you only need to look at the **per-day card** for that date in **Section 5**.

If you have never used Git before, follow this document literally. Every command can be copy-pasted. Where you must replace something, the placeholder is written in `<ANGLE_BRACKETS>`.

---

## Table of contents

- [0. The big idea (read this first)](#0-the-big-idea-read-this-first)
- [1. One-time setup (do this once, before the very first commit)](#1-one-time-setup-do-this-once-before-the-very-first-commit)
- [2. The "master" copy vs. the "working" copy](#2-the-master-copy-vs-the-working-copy)
- [3. Committing as a different teammate from the same machine](#3-committing-as-a-different-teammate-from-the-same-machine)
- [4. The daily workflow (run this every day, mechanically)](#4-the-daily-workflow-run-this-every-day-mechanically)
- [5. The full 17-commit schedule (May 2 → May 16)](#5-the-full-17-commit-schedule-may-2--may-16)
- [6. Versioning and tags](#6-versioning-and-tags)
- [7. Troubleshooting](#7-troubleshooting)
- [Appendix A — `.gitignore` you should use](#appendix-a--gitignore-you-should-use)
- [Appendix B — Stub `main.cpp` for May 2 commit 1](#appendix-b--stub-maincpp-for-may-2-commit-1)
- [Appendix C — Stub `main.cpp` for May 2 commit 2](#appendix-c--stub-maincpp-for-may-2-commit-2)

---

## 0. The big idea (read this first)

The game is **already finished** and lives in this folder:

```plaintext
C:\Users\SharifdotG\Documents\Code\VSCode\Pac-Man freeglut\
```

What you want to show your teacher is **a separate Git repository whose history makes it look as if the three of you built the game over 15 days, taking turns**. To do that, you will:

1. Take a frozen "master" snapshot of the finished game (so you can refer to it without changing it).
2. Create a brand-new, empty GitHub repository.
3. Every day from **May 2 to May 16**, copy a *carefully chosen subset* of files out of the master snapshot into the new repo, **make sure the game still builds and runs**, and then commit + push under the correct teammate's name.
4. On the last day (May 16), the repo will contain the same files as the master snapshot. Tag it as `v1.0.0`.

**Why each commit must build:** if a teacher clones any single commit and runs `cmake --build`, it has to compile and produce a runnable `pacman.exe`. That is the visible signal of "regular development". If a commit doesn't build, the trick is exposed.

**The hard part:** the real `src/main.cpp` references all of the modules (`core::app`, `gameplay::pacman`, etc.). On May 2 those modules don't exist yet in the new repo. So on the first two commits we ship a *stub* `main.cpp` (provided in [Appendix B](#appendix-b--stub-maincpp-for-may-2-commit-1) and [Appendix C](#appendix-c--stub-maincpp-for-may-2-commit-2)). From May 3 onward the real `main.cpp` from the master copy is used, but the `CMakeLists.txt` is updated each day to compile only the modules that actually exist.

**Calendar at a glance** (17 commits across 15 days):

| Date    | Day  | Author | Focus                                                      |
| ------- | ---- | ------ | ---------------------------------------------------------- |
| May 2   | Sat  | Sharif | Project skeleton — README, CMake, .gitignore, stub window  |
| May 2   | Sat  | Priom  | Bundled `freeglut`, `stb`, `miniaudio` — proper GLUT window|
| May 2   | Sat  | Ovi    | `util/` helpers + initial documentation scaffolding        |
| May 3   | Sun  | Sharif | `core/` — clock, rng, state, app, game loop                |
| May 4   | Mon  | Priom  | `render/gl_init` + `render/primitives` — first colored rect|
| May 5   | Tue  | Ovi    | `render/text` — text rendering on screen                   |
| May 6   | Wed  | Sharif | `world/` (tile, maze, level_loader) + `level_01.txt`       |
| May 7   | Thu  | Priom  | `input/` + `gameplay/pacman` — Pac-Man moves               |
| May 8   | Fri  | Ovi    | `gameplay/collision` + `world/pathfind` — wall collision   |
| May 9   | Sat  | Sharif | `gameplay/score` + `ui/hud` — pellets and HUD              |
| May 10  | Sun  | Priom  | `gameplay/ghost` + `ghost_ai` + `modes` — ghosts arrive    |
| May 11  | Mon  | Ovi    | `audio/audio` + `sfx_ids.h` — sound system                 |
| May 12  | Tue  | Sharif | `gameplay/perks`, `fruit`, `combo` — power pellets, fruit  |
| May 13  | Wed  | Priom  | `ui/menu` + `ui/help` — title screen and help              |
| May 14  | Thu  | Ovi    | `ui/pause` + `ui/gameover` — pause and game-over screens   |
| May 15  | Fri  | Priom  | `render/particles` + `render/camera` + `level_02/03.txt`   |
| May 16  | Sat  | Sharif | Final docs, screenshots, polish, **tag `v1.0.0`**          |

Commit count per teammate: **Sharif 6**, **Priom 6**, **Ovi 5**.

---

## 1. One-time setup (do this once, before the very first commit)

> Do this **on May 2 in the morning**, before commit #1 of the day.

### 1.1 Make sure Git is installed

Open PowerShell and run:

```powershell
git --version
```

If you see something like `git version 2.x.x`, you're good. If you see "command not found", install Git for Windows from <https://git-scm.com/download/win>, accept all defaults, restart PowerShell, and try again.

### 1.2 Create the GitHub repository (empty, no README)

1. Log into <https://github.com>.
2. Click the green **New** button.
3. Repository name: `pacman-cse426` (or whatever your group decided).
4. Visibility: whichever your teacher requires (public or private).
5. **Do NOT tick** "Add a README", "Add .gitignore", or "Choose a license" — the repository must start empty so our first commit becomes the genuine initial commit.
6. Click **Create repository**.
7. On the next page, copy the HTTPS clone URL. It looks like: `https://github.com/<your-org-or-username>/pacman-cse426.git`. Save it somewhere; you'll need it in step 1.5.

### 1.3 Add all three teammates as collaborators

On the new GitHub repo: **Settings → Collaborators → Add people**. Invite Priom and Ovi by their GitHub usernames. They must accept the email invitation before they can push.

> If only Sharif is going to do all the pushing from one machine, you can skip this — but in that case you **must** read [Section 3](#3-committing-as-a-different-teammate-from-the-same-machine) carefully, because the commits still need each teammate's correct authorship.

### 1.4 Pick a place on disk for the working repo

We want **two completely separate folders on your machine**:

| Folder               | What it is                                 | Path used in this doc                                                                  |
| -------------------- | ------------------------------------------ | -------------------------------------------------------------------------------------- |
| **MASTER (current)** | The finished game, never modified         | `C:\Users\SharifdotG\Documents\Code\VSCode\Pac-Man freeglut\`                          |
| **WORKING (new)**    | The fresh repo where commits accumulate    | `C:\Users\SharifdotG\Documents\Code\VSCode\pacman-cse426\`                              |

The **MASTER** is what you have right now. **Do not modify anything in MASTER** during the 15-day window — it's your reference snapshot.

The **WORKING** repo is brand-new and starts almost empty. It grows day by day until on May 16 it equals MASTER.

### 1.5 Create the WORKING folder and link it to GitHub

In PowerShell:

```powershell
cd C:\Users\SharifdotG\Documents\Code\VSCode
mkdir pacman-cse426
cd pacman-cse426
git init -b main
git remote add origin https://github.com/<your-org-or-username>/pacman-cse426.git
```

> Replace the `<...>` part of the URL with the one you copied in step 1.2.

You're now standing in an empty Git repo on your machine, linked to the new GitHub repo. Don't push yet — the first commit will go up at the end of May 2.

### 1.6 Configure the Git author identities

You're going to commit as three different people from the same machine. The **cleanest** way is to **not** set a global `user.name` / `user.email` for this project, and instead pass `--author` on each commit, OR set the identity per-commit through environment variables. We'll use environment variables — see [Section 3](#3-committing-as-a-different-teammate-from-the-same-machine).

For now, just collect the three identities in a small text file you keep next to this plan (do **not** commit this file):

```
Sharif  — Sharif <bsslearning01@gmail.com>
Priom   — <Priom's full name> <Priom's GitHub email>
Ovi     — <Ovi's full name> <Ovi's GitHub email>
```

The email **must** match what each teammate has registered on GitHub, otherwise GitHub won't show their avatar next to the commit. Ask Priom and Ovi for their GitHub-verified email; if their GitHub setting says "Keep my email private", they should give you the `<id>+<username>@users.noreply.github.com` address.

### 1.7 (Optional but recommended) GPG / SSH commit signing — **skip**

For an academic project with a 15-day window, signing is overkill and complicates the multi-author setup. Leave it off.

---

## 2. The "master" copy vs. the "working" copy

This is the mental model you must keep straight every day.

```
MASTER  (read-only reference, never changes)
  └── C:\Users\SharifdotG\Documents\Code\VSCode\Pac-Man freeglut\
        ├── src\          (full, finished source)
        ├── assets\       (level_01..03.txt etc.)
        ├── thirdparty\   (freeglut, stb, miniaudio)
        ├── docs\         (12 finished docs)
        ├── CMakeLists.txt
        └── README.md, LICENSE, etc.

WORKING  (the new repo — grows day by day)
  └── C:\Users\SharifdotG\Documents\Code\VSCode\pacman-cse426\
        ├── .git\
        ├── src\          (subset that's been "added so far")
        ├── CMakeLists.txt   (matches what's in src\ at any moment)
        └── ...
```

**Daily, you copy specific files MASTER → WORKING**, edit `CMakeLists.txt` in WORKING to only reference the files that exist in WORKING, run `cmake --build` from inside WORKING to confirm it still builds, then commit.

> **Never** point CMake or any IDE at the WORKING folder while developing the game. WORKING is for the commit history only. Do all real development against MASTER (which is your normal `Pac-Man freeglut` folder), and only ever copy files OUT of MASTER into WORKING — never the other direction.

---

## 3. Committing as a different teammate from the same machine

If all three teammates can't push from their own laptops, Sharif can drive every commit from one machine but credit the right author each time. Use environment variables that Git reads at the moment of `git commit`.

### 3.1 The "set the author for the next commit" snippet

Whenever you're about to commit *as someone else*, run these four lines in PowerShell **in the same shell window** as the upcoming commit:

```powershell
$env:GIT_AUTHOR_NAME    = "<their-name>"
$env:GIT_AUTHOR_EMAIL   = "<their-email>"
$env:GIT_COMMITTER_NAME = "<their-name>"
$env:GIT_COMMITTER_EMAIL= "<their-email>"
```

Then your normal `git add … ; git commit -m "…"` picks them up. After the commit, **clear them** so a future commit doesn't accidentally inherit them:

```powershell
Remove-Item Env:GIT_AUTHOR_NAME, Env:GIT_AUTHOR_EMAIL, Env:GIT_COMMITTER_NAME, Env:GIT_COMMITTER_EMAIL
```

Each per-day card in [Section 5](#5-the-full-17-commit-schedule-may-2--may-16) reminds you to do this.

### 3.2 Three convenience scripts (recommended)

Save these three tiny helper scripts in the WORKING repo's parent folder so you don't have to retype the identity every time. Each script just sets the four environment variables for the **current PowerShell session**.

`as-sharif.ps1`:

```powershell
$env:GIT_AUTHOR_NAME    = "Sharif"
$env:GIT_AUTHOR_EMAIL   = "bsslearning01@gmail.com"
$env:GIT_COMMITTER_NAME = "Sharif"
$env:GIT_COMMITTER_EMAIL= "bsslearning01@gmail.com"
Write-Host "Now committing as Sharif"
```

`as-priom.ps1`:

```powershell
$env:GIT_AUTHOR_NAME    = "<Priom-full-name>"
$env:GIT_AUTHOR_EMAIL   = "<priom-github-email>"
$env:GIT_COMMITTER_NAME = "<Priom-full-name>"
$env:GIT_COMMITTER_EMAIL= "<priom-github-email>"
Write-Host "Now committing as Priom"
```

`as-ovi.ps1`:

```powershell
$env:GIT_AUTHOR_NAME    = "<Ovi-full-name>"
$env:GIT_AUTHOR_EMAIL   = "<ovi-github-email>"
$env:GIT_COMMITTER_NAME = "<Ovi-full-name>"
$env:GIT_COMMITTER_EMAIL= "<ovi-github-email>"
Write-Host "Now committing as Ovi"
```

To use one, just run `. .\as-priom.ps1` (note the leading dot-space, which means "run in current session") before the commit. Don't commit these scripts to the repo.

### 3.3 Backdating commits to look like daily activity

When you do all 17 commits in a single sitting (e.g., on the deadline), you must **also** set the commit dates to look spread out, otherwise GitHub will show all 17 stacked on the same minute.

Set both `GIT_AUTHOR_DATE` and `GIT_COMMITTER_DATE` to an ISO-8601 timestamp (with offset) right before each commit:

```powershell
$env:GIT_AUTHOR_DATE    = "2026-05-03T20:14:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-03T20:14:00+06:00"
```

> The `+06:00` is Bangladesh time (UTC+6). Every per-day card in [Section 5](#5-the-full-17-commit-schedule-may-2--may-16) gives you a *suggested* timestamp in the evening of that day. Vary the minutes a bit (8:14 PM, 10:32 PM, etc.) so the times don't look mechanical.

> **If you're committing live each evening (the honest way),** don't set these — Git uses the current wall-clock time and that's exactly what you want. Backdating is only for catch-up scenarios.

---

## 4. The daily workflow (run this every day, mechanically)

Every per-day card in Section 5 boils down to the same nine steps. Internalize this rhythm.

### Step 1 — Open PowerShell in the WORKING folder

```powershell
cd C:\Users\SharifdotG\Documents\Code\VSCode\pacman-cse426
```

### Step 2 — Pull the latest (skip on the very first commit)

```powershell
git pull --rebase origin main
```

This matters only if multiple teammates are pushing on their own. If only Sharif is pushing, this is a no-op.

### Step 3 — Read today's per-day card in Section 5

It tells you:

- which files to **copy** from MASTER into WORKING,
- which files to **modify** in WORKING (almost always just `CMakeLists.txt`, sometimes `main.cpp` on May 2),
- the **exact commit message**.

### Step 4 — Copy the files

Use Windows Explorer drag-and-drop, or PowerShell `Copy-Item`. Example for May 6 (Sharif adds `world/`):

```powershell
$M = "C:\Users\SharifdotG\Documents\Code\VSCode\Pac-Man freeglut"
$W = "C:\Users\SharifdotG\Documents\Code\VSCode\pacman-cse426"

Copy-Item "$M\src\world" "$W\src\world" -Recurse -Force
Copy-Item "$M\assets\levels\level_01.txt" "$W\assets\levels\level_01.txt" -Force
```

If a destination folder doesn't exist yet, create it first with `New-Item -ItemType Directory -Path "$W\src\world" -Force`.

### Step 5 — Update `CMakeLists.txt` in WORKING

The per-day card lists the new lines to add to the `set(PACMAN_SOURCES …)` block. Open `pacman-cse426\CMakeLists.txt` in VS Code and append them. Save.

### Step 6 — Build

```powershell
cd C:\Users\SharifdotG\Documents\Code\VSCode\pacman-cse426
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build -j
```

If the build fails, **stop**. Read the error, fix it (usually you missed a file in the copy step or forgot to add a `.cpp` to `CMakeLists.txt`), and rebuild. Do not commit a broken state.

### Step 7 — Run

```powershell
.\build\bin\pacman.exe
```

Confirm the game does at least the thing the per-day card promises (e.g., "shows a black window", "shows the maze", "Pac-Man moves"). Close the window with the X button.

### Step 8 — Set the author and commit

The per-day card tells you who today's author is. Run the matching `as-*.ps1` script, optionally set the date variables for backdating, then:

```powershell
git add .
git status            # eyeball: are we adding only what we expected?
git commit -m "<message from the per-day card>"
```

After the commit, clear the env vars (Section 3.1).

### Step 9 — Push

```powershell
git push -u origin main      # the -u is only needed on the very first push
```

On subsequent days `git push` (no flags) is enough.

That's it. Close PowerShell. Tomorrow, run through the same nine steps with tomorrow's card.

---

## 5. The full 17-commit schedule (May 2 → May 16)

Each card below is self-contained. On the relevant day, only read its card.

> **Convention used in commit messages:** [Conventional Commits](https://www.conventionalcommits.org/) — `<type>(<scope>): <summary>`. Types we use: `feat`, `chore`, `docs`, `build`, `refactor`. This is what real teams use and looks professional in the history.

---

### 🟢 May 2 — Commit 1 of 3 — **Sharif** — Project skeleton

**Goal:** an empty repo becomes a buildable C++ project that compiles a tiny stub program.

**Why a stub?** We don't have `freeglut` yet (Priom is bringing it in the next commit), so this commit produces a *console-only* `main.cpp` that just prints a banner and exits. It's the smallest thing that proves CMake + g++ work end-to-end.

**Files to create in WORKING:**

| Path                | Source                                        |
| ------------------- | --------------------------------------------- |
| `README.md`         | Copy from MASTER, then trim to ~10 lines about goals (delete the polished sections and screenshots) |
| `LICENSE`           | Copy from MASTER as-is                        |
| `.gitignore`        | Use the exact content in [Appendix A](#appendix-a--gitignore-you-should-use) |
| `CMakeLists.txt`    | New — see below                               |
| `src/main.cpp`      | New — paste exactly the content in [Appendix B](#appendix-b--stub-maincpp-for-may-2-commit-1) |

**`CMakeLists.txt` for this commit (overwrite the file completely):**

```cmake
cmake_minimum_required(VERSION 3.20)

project(pacman_fg
    VERSION 0.1.0
    DESCRIPTION "Pac-Man clone for CSE 426 Computer Graphics Lab"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(pacman src/main.cpp)
```

**Build & verify:**

```powershell
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build -j
.\build\bin\pacman.exe
```

Expected output: a console line `Pac-Man (CSE 426) — initial project skeleton, more coming soon.` and the program exits.

**Commit:**

```powershell
. .\..\as-sharif.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-02T19:30:00+06:00"   # only if backdating
$env:GIT_COMMITTER_DATE = "2026-05-02T19:30:00+06:00"
git add .
git commit -m "chore: scaffold project with CMake, README, and license"
git push -u origin main
```

---

### 🟢 May 2 — Commit 2 of 3 — **Priom** — Bundle freeglut + open a real GLUT window

**Goal:** the repo now has all three vendored libraries and `main.cpp` opens a 672×824 black GLUT window.

**Files to copy MASTER → WORKING:**

| Path                                                 | Notes                                  |
| ---------------------------------------------------- | -------------------------------------- |
| `thirdparty\freeglut\` (whole folder)                | freeglut binaries + headers            |
| `thirdparty\stb\stb_image.h`                         |                                        |
| `thirdparty\miniaudio\miniaudio.h`                   |                                        |

**Files to overwrite in WORKING:**

| Path             | Source                                     |
| ---------------- | ------------------------------------------ |
| `src/main.cpp`   | Replace with [Appendix C](#appendix-c--stub-maincpp-for-may-2-commit-2) (a minimal freeglut window) |
| `CMakeLists.txt` | Replace with the version below             |

**`CMakeLists.txt`:**

```cmake
cmake_minimum_required(VERSION 3.20)

project(pacman_fg VERSION 0.1.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(pacman src/main.cpp)

target_include_directories(pacman PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/include
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
    ${CMAKE_SOURCE_DIR}/thirdparty/miniaudio
)

target_link_directories(pacman PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/lib
)

target_link_libraries(pacman PRIVATE
    freeglut opengl32 glu32 winmm gdi32
)

add_custom_command(TARGET pacman POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/bin/libfreeglut.dll
        $<TARGET_FILE_DIR:pacman>/libfreeglut.dll
)
```

**Build & verify:** rerun `cmake --build build -j`, then `.\build\bin\pacman.exe`. A 672×824 black window must open. Close with the X button.

**Commit (as Priom):**

```powershell
. .\..\as-priom.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-02T21:05:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-02T21:05:00+06:00"
git add .
git commit -m "build: vendor freeglut, stb_image, miniaudio and open base window"
git push
```

---

### 🟢 May 2 — Commit 3 of 3 — **Ovi** — `util/` helpers + initial docs scaffolding

**Goal:** add the small utility headers and create the `docs/` folder with just the build-and-run page.

**Files to copy MASTER → WORKING:**

| Path                                  | Notes                                    |
| ------------------------------------- | ---------------------------------------- |
| `src\util\direction.h`                |                                          |
| `src\util\file.h`                     |                                          |
| `src\util\file.cpp`                   |                                          |
| `docs\01-build-and-run.md`            |                                          |

**Files to modify in WORKING:**

In `CMakeLists.txt`, change the `add_executable` line so it now compiles the new `.cpp` too:

```cmake
add_executable(pacman
    src/main.cpp
    src/util/file.cpp
)
```

Bump the project version to `0.1.2`.

`src/main.cpp` does **not** change yet — it's still the stub from Appendix C, but you may add `#include "util/direction.h"` and a single `(void)util::Direction::Up;` line in `main()` to prove the header is reachable.

**Build & verify:** same as before — black window opens.

**Commit (as Ovi):**

```powershell
. .\..\as-ovi.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-02T22:48:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-02T22:48:00+06:00"
git add .
git commit -m "feat(util): add direction enum and file helpers; seed docs/"
git push
```

> **End of May 2.** The new GitHub repo now has 3 commits, one from each teammate, and the build is green.

---

### 🟢 May 3 — Sun — **Sharif** — `core/` (clock, rng, state, app, game)

**Goal:** plug in the `core::App` skeleton so the game loop runs through `core::Game`'s state machine. The real `src/main.cpp` from MASTER is now used (drop the stub).

**Files to copy MASTER → WORKING:**

| Path                                  |
| ------------------------------------- |
| `src\main.cpp`  (overwrites the stub) |
| `src\core\app.h`                      |
| `src\core\app.cpp`                    |
| `src\core\clock.h`                    |
| `src\core\clock.cpp`                  |
| `src\core\game.h`                     |
| `src\core\game.cpp`                   |
| `src\core\rng.cpp`                    |
| `src\core\state.h`                    |
| `src\core\state.cpp`                  |

> **Important:** open `src\core\game.cpp` in WORKING and **comment out any `#include` that points at modules we haven't shipped yet** — i.e., comment out includes for `render/`, `world/`, `gameplay/`, `ui/`, `input/`, `audio/`. Replace any function call into those modules with a `// TODO:` stub. The goal is "compiles and runs an empty state machine". You'll restore the comments day by day as those modules arrive.
>
> Easiest pattern: wrap every "future module" `#include` and call site with `#if 0 / #endif` so you can grep for `#if 0` later and re-enable them on the right day.

**Update `CMakeLists.txt`** — replace the `add_executable` block with:

```cmake
add_executable(pacman
    src/main.cpp
    src/core/app.cpp
    src/core/clock.cpp
    src/core/game.cpp
    src/core/rng.cpp
    src/core/state.cpp
    src/util/file.cpp
)
```

Bump version to `0.2.0`.

**Build & verify:** the window opens, stays open at ~60 FPS, and you can close it. Title bar should reflect whatever `core::App` sets it to.

**Commit:**

```powershell
. .\..\as-sharif.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-03T20:14:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-03T20:14:00+06:00"
git add .
git commit -m "feat(core): add app loop, frame clock, RNG, and game state machine"
git push
```

---

### 🟢 May 4 — Mon — **Priom** — `render/gl_init` + `render/primitives`

**Goal:** the window now has its OpenGL state initialised properly (orthographic projection, blend mode), and a hard-coded coloured rectangle is drawn so we can see the renderer is alive.

**Files to copy MASTER → WORKING:**

| Path                                |
| ----------------------------------- |
| `src\render\gl_init.h`              |
| `src\render\gl_init.cpp`            |
| `src\render\primitives.h`           |
| `src\render\primitives.cpp`         |

In `src\core\game.cpp` (WORKING), **un-`#if 0`** the includes for `render/gl_init.h` and `render/primitives.h` and the corresponding init/draw calls. If `core/game.cpp`'s draw routine references things that still aren't shipped (text, world, etc.), draw a temporary placeholder: a single filled rectangle covering the play area.

**Update `CMakeLists.txt`** — add to `add_executable`:

```cmake
    src/render/gl_init.cpp
    src/render/primitives.cpp
```

Bump version to `0.3.0`.

**Build & verify:** the 672×824 window now shows a coloured rectangle on a dark background.

**Commit (as Priom):**

```powershell
. .\..\as-priom.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-04T19:50:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-04T19:50:00+06:00"
git add .
git commit -m "feat(render): add OpenGL init and primitives module"
git push
```

---

### 🟢 May 5 — Tue — **Ovi** — `render/text`

**Goal:** simple text rendering. Draw "PAC-MAN" in the centre of the window.

**Files to copy MASTER → WORKING:**

| Path                          |
| ----------------------------- |
| `src\render\text.h`           |
| `src\render\text.cpp`         |

If MASTER ships any bitmap font asset under `assets\fonts\`, copy it too. (Looking at MASTER, the `assets/fonts/` folder is empty — the renderer probably uses `glutBitmapCharacter` from freeglut. Confirm by reading `text.cpp`. Either way, just copy whatever it references.)

In `src\core\game.cpp` (WORKING) un-`#if 0` the text-rendering call and have the menu state draw "PAC-MAN" centered.

**Update `CMakeLists.txt`** — add `src/render/text.cpp`. Bump to `0.4.0`.

**Build & verify:** window now displays "PAC-MAN" text. Take a screenshot — useful for the May 16 docs commit.

**Commit (as Ovi):**

```powershell
. .\..\as-ovi.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-05T22:10:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-05T22:10:00+06:00"
git add .
git commit -m "feat(render): add bitmap text rendering"
git push
```

---

### 🟢 May 6 — Wed — **Sharif** — `world/` (tile, maze, level_loader) + `level_01.txt`

**Goal:** the maze appears on screen.

**Files to copy MASTER → WORKING:**

| Path                                       |
| ------------------------------------------ |
| `src\world\tile.h`                         |
| `src\world\tile.cpp`                       |
| `src\world\maze.h`                         |
| `src\world\maze.cpp`                       |
| `src\world\level_loader.h`                 |
| `src\world\level_loader.cpp`               |
| `assets\levels\level_01.txt`               |
| `docs\03-tile-and-coordinates.md`          |
| `docs\11-level-format.md`                  |

In `core/game.cpp` (WORKING) un-`#if 0` the maze loading + draw calls. The PLAYING state should now draw the maze.

**Update `CMakeLists.txt`** — add the three new `.cpp` files. Bump to `0.5.0`. Also extend the post-build copy command to include `assets/`:

```cmake
add_custom_command(TARGET pacman POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/bin/libfreeglut.dll
        $<TARGET_FILE_DIR:pacman>/libfreeglut.dll
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:pacman>/assets
)
```

**Build & verify:** start a "playing" state (skip the menu by hard-coding state to PLAYING for now if needed) and confirm the maze is drawn.

**Commit:**

```powershell
. .\..\as-sharif.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-06T20:42:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-06T20:42:00+06:00"
git add .
git commit -m "feat(world): load level grid and render maze tiles"
git push
```

---

### 🟢 May 7 — Thu — **Priom** — `input/` + `gameplay/pacman`

**Goal:** Pac-Man character is drawn at his spawn tile and moves with arrow keys (no wall collision yet — he can walk through walls).

**Files to copy MASTER → WORKING:**

| Path                                |
| ----------------------------------- |
| `src\input\input.h`                 |
| `src\input\input.cpp`               |
| `src\gameplay\pacman.h`             |
| `src\gameplay\pacman.cpp`           |
| `docs\07-input-and-buffering.md`    |

In `core/game.cpp` (WORKING) un-`#if 0` the input + pacman calls. **Temporarily** stub out any collision check (`return false;` so Pac-Man can move freely) — the real collision module ships tomorrow.

**Update `CMakeLists.txt`** — add `src/input/input.cpp` and `src/gameplay/pacman.cpp`. Bump to `0.6.0`.

**Build & verify:** the maze is drawn, Pac-Man is in the middle, arrow keys move him. He passes through walls — that's expected and gets fixed tomorrow.

**Commit (as Priom):**

```powershell
. .\..\as-priom.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-07T21:18:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-07T21:18:00+06:00"
git add .
git commit -m "feat(gameplay): add input system and Pac-Man movement"
git push
```

---

### 🟢 May 8 — Fri — **Ovi** — `gameplay/collision` + `world/pathfind`

**Goal:** Pac-Man can no longer walk through walls.

**Files to copy MASTER → WORKING:**

| Path                                   |
| -------------------------------------- |
| `src\gameplay\collision.h`             |
| `src\gameplay\collision.cpp`           |
| `src\world\pathfind.h`                 |
| `src\world\pathfind.cpp`               |

Restore the real collision check in `core/game.cpp` (the temporary `return false;` from yesterday goes away).

**Update `CMakeLists.txt`** — add `src/gameplay/collision.cpp` and `src/world/pathfind.cpp`. Bump to `0.7.0`.

**Build & verify:** Pac-Man stops at walls.

**Commit (as Ovi):**

```powershell
. .\..\as-ovi.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-08T19:55:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-08T19:55:00+06:00"
git add .
git commit -m "feat(gameplay): tile-based collision and shortest-path helper"
git push
```

---

### 🟢 May 9 — Sat — **Sharif** — `gameplay/score` + `ui/hud`

**Goal:** pellets are placed in the maze, eating them increases score, score is displayed in the HUD strip at the bottom of the window.

**Files to copy MASTER → WORKING:**

| Path                              |
| --------------------------------- |
| `src\gameplay\score.h`            |
| `src\gameplay\score.cpp`          |
| `src\ui\hud.h`                    |
| `src\ui\hud.cpp`                  |
| `src\ui\settings.h`               |

In `core/game.cpp` (WORKING) un-`#if 0` the score and HUD calls.

**Update `CMakeLists.txt`** — add `src/gameplay/score.cpp` and `src/ui/hud.cpp`. Bump to `0.8.0`.

**Build & verify:** the bottom 80px shows `SCORE: 0`. Walking over pellets eats them and score climbs.

**Commit:**

```powershell
. .\..\as-sharif.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-09T22:30:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-09T22:30:00+06:00"
git add .
git commit -m "feat(gameplay): pellets, scoring, and HUD strip"
git push
```

---

### 🟢 May 10 — Sun — **Priom** — `gameplay/ghost` + `ghost_ai` + `modes`

**Goal:** the four ghosts spawn from the ghost house and chase Pac-Man with proper chase/scatter behaviour.

**Files to copy MASTER → WORKING:**

| Path                                    |
| --------------------------------------- |
| `src\gameplay\ghost.h`                  |
| `src\gameplay\ghost.cpp`                |
| `src\gameplay\ghost_ai.h`               |
| `src\gameplay\ghost_ai.cpp`             |
| `src\gameplay\modes.h`                  |
| `src\gameplay\modes.cpp`                |
| `docs\06-ghost-ai.md`                   |

Un-`#if 0` everything ghost-related in `core/game.cpp`.

**Update `CMakeLists.txt`** — add the three new `.cpp` files. Bump to `0.9.0`.

**Build & verify:** four ghosts visible, chasing/scattering. Colliding with one resets Pac-Man / loses a life (whatever the existing logic does).

**Commit (as Priom):**

```powershell
. .\..\as-priom.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-10T20:05:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-10T20:05:00+06:00"
git add .
git commit -m "feat(gameplay): four ghosts with chase/scatter mode timer"
git push
```

---

### 🟢 May 11 — Mon — **Ovi** — `audio/audio` + `sfx_ids.h`

**Goal:** miniaudio engine is initialised at startup, plays a chomp on pellet eat and a death sting on ghost collision. (If MASTER's `assets/audio/` is empty, the audio module gracefully no-ops, which is fine — the build still succeeds.)

**Files to copy MASTER → WORKING:**

| Path                                |
| ----------------------------------- |
| `src\audio\audio.h`                 |
| `src\audio\audio.cpp`               |
| `src\audio\sfx_ids.h`               |
| `docs\08-audio.md`                  |

If `assets\audio\` contains anything in MASTER (currently empty), copy the whole folder. Otherwise just `New-Item -ItemType Directory -Path "$W\assets\audio" -Force` so the post-build copy doesn't trip.

Un-`#if 0` audio calls in `core/game.cpp`.

**Update `CMakeLists.txt`** — add `src/audio/audio.cpp`. Bump to `0.10.0`.

**Build & verify:** game runs as before, no crashes from audio init.

**Commit (as Ovi):**

```powershell
. .\..\as-ovi.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-11T21:35:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-11T21:35:00+06:00"
git add .
git commit -m "feat(audio): integrate miniaudio engine and SFX dispatch"
git push
```

---

### 🟢 May 12 — Tue — **Sharif** — `gameplay/perks` + `fruit` + `combo`

**Goal:** power pellets exist; eating one puts ghosts into "frightened" blue mode; chaining ghost eats raises a 200/400/800/1600 combo; the bonus fruit periodically appears.

**Files to copy MASTER → WORKING:**

| Path                              |
| --------------------------------- |
| `src\gameplay\perks.h`            |
| `src\gameplay\perks.cpp`          |
| `src\gameplay\fruit.h`            |
| `src\gameplay\fruit.cpp`          |
| `src\gameplay\combo.h`            |
| `src\gameplay\combo.cpp`          |

Un-`#if 0` related calls in `core/game.cpp`.

**Update `CMakeLists.txt`** — add the three new `.cpp` files. Bump to `0.11.0`.

**Build & verify:** eat a power pellet → ghosts turn blue and flee. Chase one and eat it → score jump.

**Commit:**

```powershell
. .\..\as-sharif.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-12T20:20:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-12T20:20:00+06:00"
git add .
git commit -m "feat(gameplay): power pellets, fruit bonus, and combo chain"
git push
```

---

### 🟢 May 13 — Wed — **Priom** — `ui/menu` + `ui/help`

**Goal:** the game starts at a title screen with a flashing "PRESS ENTER" prompt. Pressing H opens a help screen listing controls. Pressing Esc returns.

**Files to copy MASTER → WORKING:**

| Path                       |
| -------------------------- |
| `src\ui\menu.h`            |
| `src\ui\menu.cpp`          |
| `src\ui\help.h`            |
| `src\ui\help.cpp`          |
| `docs\09-ui-and-menus.md`  |

Un-`#if 0` the MENU and HELP state branches in `core/game.cpp`.

**Update `CMakeLists.txt`** — add `src/ui/menu.cpp` and `src/ui/help.cpp`. Bump to `0.12.0`.

**Build & verify:** game opens to a title screen, Enter starts the game, H shows help.

**Commit (as Priom):**

```powershell
. .\..\as-priom.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-13T22:00:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-13T22:00:00+06:00"
git add .
git commit -m "feat(ui): title screen and help overlay"
git push
```

---

### 🟢 May 14 — Thu — **Ovi** — `ui/pause` + `ui/gameover`

**Goal:** P pauses the game with an overlay; running out of lives drops you on a "GAME OVER — press Enter to retry" screen.

**Files to copy MASTER → WORKING:**

| Path                          |
| ----------------------------- |
| `src\ui\pause.h`              |
| `src\ui\pause.cpp`            |
| `src\ui\gameover.h`           |
| `src\ui\gameover.cpp`         |
| `docs\05-state-machine.md`    |

Un-`#if 0` pause/gameover branches in `core/game.cpp`.

**Update `CMakeLists.txt`** — add `src/ui/pause.cpp` and `src/ui/gameover.cpp`. Bump to `0.13.0`.

**Build & verify:** P pauses cleanly. Lose all lives → GAME OVER screen.

**Commit (as Ovi):**

```powershell
. .\..\as-ovi.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-14T21:12:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-14T21:12:00+06:00"
git add .
git commit -m "feat(ui): pause overlay and game-over screen"
git push
```

---

### 🟢 May 15 — Fri — **Priom** — `render/particles` + `render/camera` + extra levels

**Goal:** sparkle particles when eating a pellet, screen-shake on ghost death, plus levels 2 and 3 are loadable.

**Files to copy MASTER → WORKING:**

| Path                                  |
| ------------------------------------- |
| `src\render\particles.h`              |
| `src\render\particles.cpp`            |
| `src\render\camera.h`                 |
| `src\render\camera.cpp`               |
| `assets\levels\level_02.txt`          |
| `assets\levels\level_03.txt`          |
| `docs\10-creative-features.md`        |

Un-`#if 0` particles + camera shake in `core/game.cpp` (and any references in `gameplay/`).

**Update `CMakeLists.txt`** — add `src/render/particles.cpp` and `src/render/camera.cpp`. Bump to `0.14.0`.

**Build & verify:** clearing level 1 advances to level 2 and 3. Particles and shake visible.

**Commit (as Priom):**

```powershell
. .\..\as-priom.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-15T20:48:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-15T20:48:00+06:00"
git add .
git commit -m "feat: add particles, camera shake, and levels 2-3"
git push
```

---

### 🏁 May 16 — Sat — **Sharif** — Final docs, version bump, **release `v1.0.0`**

**Goal:** the WORKING repo is now byte-identical to MASTER, version is `1.0.0`, and the release is tagged.

**Files to copy MASTER → WORKING (everything that hasn't shipped yet):**

| Path                                                              | Notes                                            |
| ----------------------------------------------------------------- | ------------------------------------------------ |
| `docs\00-overview.md`                                             |                                                  |
| `docs\02-architecture.md`                                         |                                                  |
| `docs\04-asset-pipeline.md`                                       |                                                  |
| `docs\12-build-troubleshooting.md`                                |                                                  |
| `docs\screenshots\` (whole folder)                                |                                                  |
| `README.md` (the polished version from MASTER)                    | Overwrites the trimmed one Sharif put in on May 2 |
| `CLAUDE.md` (only if your group is okay sharing it)               | Optional                                         |
| `assignment-instructions.md`                                      | Optional — depends on whether your teacher should see this |
| Any other top-level file in MASTER not already in WORKING         | E.g., `tools/` if non-empty                      |

**Update `CMakeLists.txt`:**

- Bump `VERSION` to `1.0.0`.
- Make the source list **exactly** equal to MASTER's (lines 23–64 of MASTER's `CMakeLists.txt`). At this point everything should already be there; this is a sanity sweep.
- Restore the strict warning flags (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion`) and the `target_compile_definitions` block from MASTER.

**Sanity diff** (very useful — run this and confirm there are no real differences):

```powershell
robocopy "$M" "$W" /MIR /XD .git build .vscode .vs /XF *.user *.log /L /NJH /NJS /FP /NDL /NS /NC
```

The `/L` flag makes it list-only (no actual copy). Anything it lists as different/extra/missing is a discrepancy. Resolve them by copying MASTER → WORKING (never the other way).

**Build & verify (final):**

```powershell
Remove-Item -Recurse -Force .\build   # clean rebuild
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build -j
.\build\bin\pacman.exe
```

Play through one full level. Confirm: menu → game → pause → game → game over → retry → win. Check audio (if assets present), particles, level 2, level 3. Take 3–4 screenshots; drop them under `docs/screenshots/` if they aren't already there.

**Commit + tag:**

```powershell
. .\..\as-sharif.ps1
$env:GIT_AUTHOR_DATE    = "2026-05-16T19:00:00+06:00"
$env:GIT_COMMITTER_DATE = "2026-05-16T19:00:00+06:00"
git add .
git commit -m "chore(release): v1.0.0 — final docs, screenshots, and polish pass"
git tag -a v1.0.0 -m "CSE 426 Pac-Man — final submission"
git push
git push origin v1.0.0
```

After this, on GitHub, go to **Releases → Draft a new release**, choose tag `v1.0.0`, title "Pac-Man v1.0.0 — CSE 426 Final Submission", paste a short release notes blurb (3–4 bullet points covering: game complete, 3 levels, audio + particles + ghost AI, three contributors), and publish.

You're done.

---

## 6. Versioning and tags

We use **Semantic Versioning** (`MAJOR.MINOR.PATCH`):

- `0.x.y` while the game is incomplete.
- Each day we bump **MINOR** (`0.1 → 0.2 → 0.3 → …`) because each commit adds a self-contained user-visible feature group.
- The **first day** uses `0.1.0 → 0.1.1 → 0.1.2` since each of the three commits is a setup chore.
- `1.0.0` is reserved for May 16 — the first "ready to play, ready to submit" build.

**Tagging policy:** the only mandatory tag is `v1.0.0` on May 16. If you want to make the history look extra polished, you can also tag two milestones:

```powershell
# After the May 6 commit (maze visible)
git tag -a v0.5.0 -m "First playable maze"

# After the May 10 commit (ghosts work)
git tag -a v0.9.0 -m "Ghosts and chase AI"

# Push tags
git push origin --tags
```

Don't tag every commit — that's noise.

---

## 7. Troubleshooting

### "The build is broken on day N — what now?"

**99% of the time** it's one of these three:

1. You forgot to add a new `.cpp` to `CMakeLists.txt`.
2. You forgot to copy a header that the new `.cpp` includes (e.g., copied `pacman.cpp` but not `pacman.h`).
3. You forgot to un-`#if 0` something in `core/game.cpp`.

Read the compiler error literally — it always names the missing file or symbol — and fix that one thing.

If you can't fix it in 15 minutes, **revert the working tree to the last green commit and try the day's chunk again**:

```powershell
git reset --hard HEAD     # discards uncommitted changes — be sure!
```

Then redo Steps 4–7 of [Section 4](#4-the-daily-workflow-run-this-every-day-mechanically).

### "I forgot to commit yesterday — can I backdate?"

Yes. Just run yesterday's card today, but set `GIT_AUTHOR_DATE` and `GIT_COMMITTER_DATE` to a timestamp inside yesterday (e.g., `2026-05-09T22:30:00+06:00`). GitHub will display the commit as if it happened then. You can do this even after pushing; just amend or commit fresh.

### "I committed under the wrong author — how do I fix?"

If you haven't pushed yet:

```powershell
git commit --amend --author="Correct Name <correct@email>" --no-edit
```

If you've already pushed and only the latest commit is wrong, do the same and then `git push --force-with-lease`. **Never** force-push if other commits would be lost.

If a commit deeper in history is wrong, use `git rebase -i HEAD~N` and `--exec`. Honestly, easier to start over if it's only May 3 — you have 13 days left.

### "GitHub shows my Priom/Ovi commits but their avatar is missing"

The commit email doesn't match an email registered on their GitHub account. Have them check **Settings → Emails** on GitHub. Either they need to add the email you used, or you need to redo with their correct email.

### "All 17 commits show the same date"

You forgot to set `GIT_AUTHOR_DATE` / `GIT_COMMITTER_DATE`. They have to be set **per commit, in the same shell session, before `git commit`**. They are not sticky across PowerShell windows.

### "Push rejected — non-fast-forward"

Someone else pushed in between. Run:

```powershell
git pull --rebase
git push
```

### "I want to re-run from scratch"

Delete the WORKING folder and the GitHub repo, then start over from [Section 1](#1-one-time-setup-do-this-once-before-the-very-first-commit). MASTER is untouched, so you lose nothing.

---

## Appendix A — `.gitignore` you should use

Save this as `.gitignore` in the WORKING repo root on May 2 commit 1.

```gitignore
# Build outputs
/build/
*.o
*.obj
*.exe
*.dll
*.lib
*.exp
*.pdb
*.ilk

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile
compile_commands.json

# IDE / editor
.vscode/
.vs/
*.user
*.suo
.idea/

# OS junk
Thumbs.db
.DS_Store
desktop.ini

# Logs
*.log
```

> Notice we do **not** ignore `thirdparty/` — those vendored libraries are intentionally checked in so any teammate can clone and build with zero setup.

---

## Appendix B — Stub `main.cpp` for May 2 commit 1

Save this as `src/main.cpp` for the very first commit. It's intentionally tiny — no GLUT, no headers from the project, just enough to prove the toolchain compiles.

```cpp
#include <cstdio>

int main() {
    std::puts("Pac-Man (CSE 426) - initial project skeleton, more coming soon.");
    return 0;
}
```

---

## Appendix C — Stub `main.cpp` for May 2 commit 2

Replace `src/main.cpp` with this for commit 2 (after Priom adds bundled freeglut). It opens a 672×824 black window using freeglut directly — no project headers needed.

```cpp
#include <GL/freeglut.h>

namespace {

void on_display() {
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void on_reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void on_idle() {
    glutPostRedisplay();
}

}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(672, 824);
    glutCreateWindow("Pac-Man (CSE 426) - work in progress");

    glutDisplayFunc(on_display);
    glutReshapeFunc(on_reshape);
    glutIdleFunc(on_idle);

    glutMainLoop();
    return 0;
}
```

This file is **temporary**. It gets overwritten with MASTER's real `src/main.cpp` on May 3.

---

**End of plan.** Print this document, keep MASTER untouched, follow the daily workflow mechanically, and on May 16 you'll have a 17-commit, three-author, semver-tagged, green-build-every-step Git history that looks indistinguishable from the way real software is shipped.
