---
name: verify-phase
description: Run the manual verify checklist at the end of an implementation phase, capture a screenshot, and update the matching docs/NN-*.md.
---

# verify-phase

Use this skill at the **end** of every phase (P0–P11). The codebase has no
unit tests — verification is manual + visual. Skipping this is how regressions
sneak in.

## When to invoke

After all code changes for a phase are written and you believe the phase is
complete. Before you tell the user "phase done."

## Steps

1. **Build cleanly:**

   ```bash
   cmake --build build -j
   ```

   If the build fails, fix it. Do not proceed.

2. **Launch the exe:**

   ```bash
   ./build/bin/pacman.exe
   ```

   You may need `start ./build/bin/pacman.exe` from PowerShell or just
   double-click from Explorer if `glutMainLoop` blocks the terminal.

3. **Run the phase's verify checklist** from the implementation plan
   (`C:\Users\SharifdotG\.claude\plans\hey-claude-the-given-golden-lobster.md`).
   Each phase has 3–6 specific things to confirm — walk them all.

4. **Take a screenshot** of the running game showing the new capability.
   Save to `docs/screenshots/PNN.png` where NN is the phase number, zero-padded
   (e.g. `P00.png`, `P05.png`).

5. **Update `docs/NN-*.md`** — the doc that matches this phase's feature. Cover:
   - What it does (1-paragraph summary)
   - Where it lives (file:line references using markdown links)
   - How to tweak it (knobs / constants / env vars)
   - Any gotchas hit during implementation

   If a doc didn't exist yet, create it. Use the file numbering from
   `docs/00-overview.md`.

6. **Update `docs/00-overview.md`**: flip this phase's status to "complete" in
   the phase table.

7. **Tell the user the phase is done.** Do NOT commit (per CLAUDE.md). The user
   handles all git operations.

## Outputs

- `build/bin/pacman.exe` runs cleanly.
- `docs/screenshots/PNN.png` exists.
- `docs/NN-*.md` is updated (or created).
- `docs/00-overview.md` phase table is updated.
- A short summary message to the user listing what was done + verify-checklist
  results.
