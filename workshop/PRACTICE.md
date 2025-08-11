## Cursor + LLM Practice Pack: Repetition Drills for Potatogame

This practice pack provides short, focused drills to build fluency using Cursor with an LLM on this C++ codebase. Use these to get lots of repetition beyond the 1‑hour workshop.

### How to use
- Pick 1–2 sets per session (30–60 minutes)
- For each task, first ask the LLM for a plan, then request minimal edits, then review and apply
- Build and run after each logical change

### Ground rules
- Keep changes small, reversible, and readable
- Prefer early returns, descriptive names, and isolated helpers
- Use NZ spelling in comments and docs
- Keep files under ~500 lines; extract helpers if needed

---

## Prompt warm‑ups (5–10 minutes)
Paste into Cursor to quickly map the code before editing.

```
Summarise the game loop: where are per‑frame updates and rendering called? Cite exact functions in Game.cpp/Game.h and any other files.
```

```
List all keyboard input handling points and what actions they trigger. Note which are edge‑triggered vs held.
```

```
Identify where enemies are spawned and updated. Show the timers or conditions controlling spawn frequency.
```

```
Find any long functions (>60 lines) I’m likely to touch. Propose a safe extraction plan that keeps behaviour identical.
```

```
Point out any magic numbers involved in movement speeds, fire rates, or spawn intervals. Suggest named constants and where to define them.
```

---

## Set A — Micro refactors (choose 2–3)

### A1. Name the magic numbers
Goal: Replace a few obvious magic numbers with named constants.

Prompt:
```
Find the top 3 magic numbers used in movement, firing, or spawning that would benefit from named constants. Propose names, types, and exact insertion points (header vs source). Generate minimal edits.
```

Checklist:
- Compiles and runs unchanged in behaviour
- Names are descriptive and scoped sensibly

### A2. Extract a helper from the update loop
Goal: Reduce a long branch in the per‑frame update into a helper function.

Prompt:
```
Locate a cohesive block inside the per‑frame update (e.g., bullet–enemy collision handling) and extract it into a static/internal helper with a clear name and parameters. Show call‑site edit + new function body.
```

Checklist:
- Original function shorter and clearer
- No change in behaviour

### A3. Introduce early returns
Goal: Flatten nesting by turning guard conditions into early returns.

Prompt:
```
Identify a function with nested if/else. Rewrite with early returns to reduce indentation and improve readability. Keep semantics identical. Provide the minimal diff.
```

### A4. Improve variable names
Goal: Rename 2–3 unclear variables to descriptive names.

Prompt:
```
Suggest safer, more descriptive names for 2–3 variables in Game.cpp that hurt readability. Show the rename edits and any impacted call sites. Avoid broad renames; keep the diff small.
```

### A5. Lightweight docs
Goal: Add brief function‑level docstrings for non‑obvious functions.

Prompt:
```
Pick 2 non‑obvious functions I touch and add concise docstrings explaining purpose, inputs, and side effects. Keep them short and above the function.
```

---

## Set B — Behaviour flags (choose 1–2)

### B1. Hold to sprint
Goal: Holding Shift temporarily increases player speed; release to return to normal.

Prompt:
```
Add a 'sprint' modifier when Left Shift is held: increase player speed by a small factor while held. Identify input handling, where speed is used, and insert a minimal guard. Provide small, localised edits.
```

Acceptance:
- Speed boost only while held; no drift in state

### B2. No‑spawn debug toggle
Goal: Toggle a debug flag (e.g., key 'O') to temporarily halt enemy spawns.

Prompt:
```
Introduce a boolean debug flag toggled by 'O' that stops new enemy spawns while active. Ensure existing enemies continue updating. Show precise edits in spawn logic and input handling.
```

### B3. Gentle difficulty scaler
Goal: Every N seconds, slightly increase enemy speed.

Prompt:
```
Implement a small difficulty scaler that increases enemy speed by a tiny factor every 30 seconds of gameplay. Keep constants named and changes minimal. Show where to track elapsed time and where to apply the multiplier.
```

### B4. Bullet cap
Goal: Limit simultaneous bullets to a small maximum.

Prompt:
```
Add a MAX_BULLETS cap. When at cap, skip firing new bullets. Identify where bullets are created and the container storing them; add the minimal guard.
```

---

## Set C — Reading and tracing (choose 2)

### C1. Call path sketch
```
Given function X in Game.cpp (you choose an important one), show who calls it, what it calls, and any side effects. Create a short call tree sketch with file:line refs for navigation.
```

### C2. Update frequency audit
```
List all per‑frame update entry points and their responsibilities. Identify any duplicated work or ordering assumptions that could cause subtle bugs.
```

### C3. Spawn lifecycle
```
Trace the full lifecycle of an enemy from spawn to removal. Cite the exact functions that create, update, collide, and destroy.
```

---

## Set D — Debugging drills (choose 1–2)

### D1. Sticky key prevention
```
If key toggles are firing multiple times due to key repeat, propose an edge‑trigger approach (detect key down transitions) and show minimal edits to implement it for one toggle.
```

### D2. Frame‑time spike checklist
```
Provide a checklist to diagnose a frame‑time spike: large containers, per‑frame allocations, expensive collision checks, or unnecessary copies. Point to hot spots in this codebase that might be culprits.
```

### D3. Spawn halt after 2 minutes
```
Hypothesis: enemy spawns stop after ~120s. List plausible timer/overflow/conditon issues to check in this codebase and where to instrument logs/asserts to confirm.
```

---

## Set E — Small visuals (choose 1)

### E1. Simple overlay utility
```
Add a tiny overlay helper for future UI (e.g., translucent rectangle + centred text). Use the project’s existing rendering approach; keep it minimal. Show where it would be called.
```

### E2. Hit flash hint
```
When the player takes damage, briefly show a subtle screen‑tint or a short‑lived flag you can later render. Keep logic tiny and self‑contained.
```

---

## Build and run loop (PowerShell)

```powershell
# From repo root
mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Debug; cd ..
```

After each small change: build; run; manually verify behaviour; revert if the change bloats complexity.

---

## Self‑check rubric
- Edits are small and easy to review
- Behaviour changes are intentional and testable
- Names are clear; magic numbers are minimised
- Long functions are shortened by extraction
- Files remain under ~500 lines

---

## Extra homework (pick 1–3)
- Input remapping scaffold
  - Centralise key bindings in one place; use named actions instead of raw keys
- Difficulty knobs
  - Gather spawn rates and speed scalers into a small config struct
- Visual polish
  - Add a basic pause overlay or a simple health bar using existing drawing utilities
- Code health
  - Add lightweight docstrings to complex functions you touch

### Homework prompts
```
Propose a minimal input remapping scaffold with a single source of truth for key bindings. Replace direct key checks with named actions for one feature.
```

```
Identify parameters that control difficulty (spawn rate, speeds). Propose a small Config struct and show how to thread it through without large diffs.
```

```
Recommend the smallest possible visual element (overlay or bar) that fits the current rendering approach. Provide the precise edits.
```

