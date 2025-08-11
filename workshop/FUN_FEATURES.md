## Workshop Feature Menu for Potatogame

Choose one feature to implement during your session. Each feature includes the files you'll likely need to modify and specific acceptance criteria to validate your work.

### Beginner features 
- Dash (short burst on key press)
  - Touch: `Player.*`, `Game.*`
  - Done when: press a key to lunge a short distance with brief cooldown; no clipping through walls if collision exists.

- XP magnet (auto‑pull nearby orbs)
  - Touch: `ExperienceOrb.*`, `Game.*`
  - Done when: orbs within small radius accelerate toward the player.

- Invulnerability on level‑up (grace period)
  - Touch: `Player.*`, `Game.*`
  - Done when: a short timer prevents damage and gives a subtle visual hint.

- Bullet cap (prevent spam)
  - Touch: `Weapon.*`, `Bullet.*`, `Game.*`
  - Done when: at max bullets, firing attempts are skipped.

- Screenshot mode (hide UI)
  - Touch: `Game.*`
  - Done when: toggle hides HUD/overlays; gameplay unaffected.

- Low‑health warning (flash or sound)
  - Touch: `Player.*`, `Game.*`
  - Done when: at <20% HP a subtle on‑screen indicator appears.

- Slow‑mo on last enemy (brief)
  - Touch: `Game.*`, `Enemy.*`
  - Done when: when only one enemy remains, time scale eases down then restores on spawn.

### Intermediate features 
- Sprint while held (temporary speed boost)
  - Touch: `Player.*`, `Game.*`
  - Done when: holding Shift increases speed; release restores normal speed.

- Ricochet bullets (limited bounces)
  - Touch: `Bullet.*`, `Game.*`
  - Done when: bullets bounce off bounds a fixed number of times.

- Piercing shots (pass through N enemies)
  - Touch: `Bullet.*`, `Enemy.*`
  - Done when: bullet tracks remaining pierces; disappears after N hits.

- Combo multiplier (reward streaks)
  - Touch: `Game.*`
  - Done when: consecutive kills within a window increase score multiplier that decays over time.

- Shield pickup (one‑time block)
  - Touch: `Player.*`, `Game.*`
  - Done when: next hit consumes shield instead of HP; small feedback effect.

- Shop reroll button (new choices)
  - Touch: `Shop.*`
  - Done when: reroll cost applied; new set generated; prevents duplicate purchase bugs.

- Enemy: Splitter (splits on death)
  - Touch: `Enemy.*`, `Game.*`
  - Done when: on death, spawns two smaller enemies with adjusted stats.

### Advanced features 
- Boss wave (simple telegraphed attack)
  - Touch: `Game.*`, `Enemy.*`
  - Done when: after a timer, a high‑HP enemy spawns with a predictable attack pattern.

- Status effect: Burn (DoT)
  - Touch: `Enemy.*`, `Bullet.*`
  - Done when: hits can apply burn; enemies take periodic damage for a few ticks.

- Time‑freeze pickup (brief stop)
  - Touch: `Game.*`, `Enemy.*`, `Bullet.*`
  - Done when: enemies/bullets stop for a short duration; UI shows a timer.

- Pet companion (follows and pings)
  - Touch: `Player.*`, `Game.*`
  - Done when: small entity follows the player and periodically pings nearby enemies (e.g., tiny damage or mark).

- Radar/minimap dot sweep
  - Touch: `Game.*`
  - Done when: minimal top‑corner radar shows approximate enemy/player positions.

- Environmental hazard: Spike ring
  - Touch: `Game.*`
  - Done when: rotating spikes around arena centre deal damage on contact.

- Loot chest drop (random reward)
  - Touch: `Game.*`, `Shop.*`
  - Done when: rare chest spawns; opening grants a small random bonus.

### Debug and utility features 
- Debug: Toggle spawn off
  - Touch: `Game.*`
  - Done when: key toggle disables new enemy spawns; existing enemies continue.

- Frame time HUD (very simple)
  - Touch: `Game.*`
  - Done when: average frame time or FPS shown in a corner.

- Seeded run (repeatable randomness)
  - Touch: `Game.*`
  - Done when: entering a numeric seed makes enemy/shop RNG deterministic.

### Implementation workflow
- "Show me where [feature area] is updated per frame and where input is handled. Cite exact functions and files."
- "Propose the smallest possible edit plan to add [feature], listing precise functions to change and any new fields."
- "Generate minimal diffs for the plan with early returns and named constants. Keep files under 500 lines; extract helpers if needed."
- "List edge cases for this feature and a quick test checklist I can run manually."

### Validation checklist
- Behaviour is testable and visible
- Build succeeds; no regressions in basic play
- Names are clear; magic numbers reduced
- Change is localised and reversible

