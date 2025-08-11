## Cursor + LLM Workshop: Add a Feature to Potatogame

This guided workshop teaches you how to use Cursor with an LLM to read unfamiliar C++ code, plan an edit, and ship a working feature in about one hour. It finishes with optional homework tasks to deepen your skills.

### Goals
- Understand the repository layout and game loop at a high level
- Practise asking effective questions and prompts in Cursor
- Implement a small, well‑defined feature end‑to‑end
- Validate your change by building and running locally

### Who this is for
- Beginners to intermediate developers new to AI‑assisted coding
- Anyone wanting to quickly add features to an existing C++ codebase

### Repo at a glance
- Source: `src/` (key files: `Game.cpp`, `Player.cpp`, `Enemy.cpp`, `Shop.cpp`)
- Build system: `CMakeLists.txt`
- Entry point: `src/main.cpp`

## 60‑minute agenda
1) 0–5 min — Orientation and success criteria
2) 5–15 min — Read the code with Cursor + LLM
3) 15–40 min — Implement the core feature (guided prompts)
4) 40–55 min — Test, debug, and iterate
5) 55–60 min — Reflect and next steps

## Success criteria (Definition of Done)
- You can toggle the new feature at runtime and observe visible behaviour changes
- The project builds successfully via CMake or your IDE
- Your change is small, readable, and isolated to relevant files
- You can explain what you changed and why

## Setup (Windows / PowerShell)
You can use Visual Studio, VS Code, or Cursor directly. If building via CMake from the terminal:

```powershell
# From repo root
mkdir build; cd build; cmake .. -G "Visual Studio 17 2022"; cmake --build . --config Debug
```

Tip: Open the folder in Cursor, then use inline chat to ask about specific files or selections. Prefer small, precise questions.

## Feature menu (pick one)
Choose one primary feature for the workshop. Each option includes focused prompts you can paste into Cursor.

### Option A — Pause and Resume (recommended for 1 hour)
Add a pause toggle so the game state freezes and displays a simple overlay.

- Expected behaviour
  - Press `P` to toggle pause on/off
  - While paused: stop enemy movement/spawn, stop bullets updating, draw a semi‑transparent overlay with “Paused”
  - While not paused: game behaves as before

- Likely files
  - `src/Game.h`, `src/Game.cpp` (game loop, input handling, draw)
  - `src/Enemy.cpp` and `src/Bullet.cpp` (updates may need a pause check)

- Acceptance criteria
  - No updates or spawns occur while paused
  - Overlay visibly indicates paused state

- Prompts to use in Cursor
  1. Context mapping
     ```
     I am adding a Pause/Resume feature.
     Show me where the main update loop, render loop, and keyboard input are handled. Cite specific functions in Game.cpp/Game.h and any other files involved.
     ```
  2. Design
     ```
     Propose the smallest change to add a boolean paused state to the game, wiring it to the 'P' key. Outline:
     - where to store paused state
     - where to handle key toggle (edge trigger, not repeat)
     - where to short‑circuit updates and spawns while paused
     - where to draw a semi‑transparent overlay with the text 'Paused'
     Provide a step‑by‑step edit plan referencing exact files and functions.
     ```
  3. Implementation
     ```
     Based on that plan, generate the minimal diffs for:
     - Game.h: add paused field and any function signatures
     - Game.cpp: initialise paused, handle 'P' key toggle, skip update paths while paused, draw overlay
     Suggest small, readable edits only. Keep files under 500 lines by extracting helpers if needed.
     ```
  4. Guarding updates
     ```
     Identify every place enemies, bullets, and spawns update each frame. Insert early returns or guards when paused so no movement, spawns, or physics occur. Show the precise edits by file and function.
     ```
  5. Visual overlay
     ```
     Add a simple semi‑transparent overlay and 'Paused' text in the draw/render step. If there is no text/overlay helper, propose the simplest placeholder using the existing rendering approach in this project.
     ```
  6. Sanity checks
     ```
     Double‑check for edge cases: rapid toggling, initial state, focus lost, and ensuring we resume cleanly. Propose fixes if needed.
     ```

### Option B — Player Health Bar and Hit Feedback
Draw a player health bar and add brief visual feedback when the player is hit.

- Expected behaviour
  - A simple bar shows current HP vs max HP
  - On hit, briefly flash a red tint or scale the bar for feedback

- Likely files
  - `src/Player.cpp`, `src/Player.h` (health)
  - `src/Game.cpp` (draw)

- Prompts to use in Cursor
  1. ```
     Where is player health stored and modified? Point me to the exact variables and functions in Player.* and any callers.
     ```
  2. ```
     Propose a minimal rendering approach to draw a simple HP bar using the project's existing draw/raster methods. Include where to call it and how to compute the bar width.
     ```
  3. ```
     Generate small edits to implement HP bar drawing and a brief hit feedback effect. Keep changes local and simple.
     ```

### Option C — New Enemy Variant: Runner
Introduce a fast, low‑health enemy that occasionally spawns and rushes the player.

- Expected behaviour
  - Runner moves faster than standard enemies but has reduced health
  - Spawns at intervals after a certain time or wave

- Likely files
  - `src/Enemy.cpp`, `src/Enemy.h` (behaviour)
  - `src/Game.cpp` (spawning)

- Prompts to use in Cursor
  1. ```
     Show me how enemies are defined, updated, and spawned. Identify the class(es) and functions involved, citing exact symbols in Enemy.* and Game.*.
     ```
  2. ```
     Propose a minimal approach to add a 'Runner' variant: either via subclassing, a type enum with behaviour switch, or parameterised factory. Recommend the smallest and clearest fit for this codebase.
     ```
  3. ```
     Generate the edits to add Runner with tuned speed/HP and spawn rules. Keep the diff small and readable.
     ```

## Working with Cursor effectively
- Ask one question at a time; prefer concrete, file‑anchored prompts
- Select code and use inline chat to get precise explanations
- Request edit plans first, then code suggestions; review before applying
- Keep each change small; build and run after each logical step

### Quick diagnostic prompts
- "Summarise how input is handled in `Game.cpp`. Where are keys read and when are actions triggered?"
- "List all per‑frame update entry points (functions called every tick) and who calls them."
- "Identify where enemies are spawned and on what timers."

## Build, run, and validate
After applying edits, rebuild and run. From PowerShell:

```powershell
cd build; cmake --build . --config Debug; cd ..
```

Manually verify your acceptance criteria for the selected feature.

## Reflection (5 minutes)
- What did Cursor get right away vs what needed refining?
- Which prompt phrasing worked best?
- How would you further simplify your changes?

## Homework (choose 1–3)
- Polish your feature
  - For Pause: add a subtle vignette or a greyscale effect while paused; add a pause menu with “Resume” and “Quit”
  - For HP Bar: animate damage/heal with eased interpolation; add numeric labels
  - For Runner: add spawn scaling based on time survived

- Add input remapping
  - Introduce a simple key binding config and use it for your feature’s input

- Improve code structure
  - Extract small helper functions to keep files under 500 lines
  - Add clear names and early returns to reduce nesting

- Document your work
  - Create a short section in a new `workshop/NOTES.md` explaining what you changed, where, and why

- Stretch: Gameplay balance
  - Make spawn rate or difficulty ramp configurable from a single place (constant or config struct), and explain the trade‑offs

### Homework prompts
Copy/paste as needed in Cursor:

``` 
Propose a refactor that extracts helpers for any function over ~60 lines I touched for the Pause feature. Keep names descriptive and prefer early returns. Show the exact edits.
```

```
Suggest a minimal input remapping approach for this codebase. Identify the single source of truth for key bindings and replace direct key checks with named actions.
```

```
Recommend a simple visual polish for the Pause overlay that fits the current rendering utilities. Provide a tiny, readable diff.
```

## Facilitation tips (for instructors)
- Timebox reading to 10 minutes; move quickly into implementation
- Encourage learners to ask Cursor for a plan first, not just code
- Celebrate small, visible wins; don’t over‑engineer in the hour

---

Kia ora and happy hacking! Keep changes focused, readable, and reversible. 

