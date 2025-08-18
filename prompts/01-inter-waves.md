# Inter-Wave Transition System Requirements

## Overview
Implement a proper inter-wave transition system that provides clear feedback to players about wave completion, clears enemies between waves, and adds appropriate pauses for better pacing.

## Current Issues
1. **No enemy cleanup**: Enemies from previous wave persist into the next wave
2. **Immediate shop opening**: Shop opens instantly when wave timer expires with no transition
3. **No completion feedback**: No visual indication that a wave has completed successfully  
4. **Immediate wave start**: Next wave starts immediately after shop closes with no preparation time
5. **Poor pacing**: No breathing room between intense combat and shop management

## Proposed Solution: Wave Transition State Machine

### New Game States
Add a new enum `GameState` to manage wave transitions:
```cpp
enum class GameState {
    WAVE_ACTIVE,        // Normal wave gameplay
    WAVE_COMPLETED,     // Wave just ended, showing completion message
    SHOP_OPENING,       // Brief pause before shop opens
    SHOP_ACTIVE,        // Shop is open for player interaction
    SHOP_CLOSING,       // Brief pause after shop closes
    WAVE_STARTING       // Brief pause before next wave begins
};
```

### Transition Flow
1. **WAVE_ACTIVE** → **WAVE_COMPLETED** (when `waveTimer >= waveDuration`)
2. **WAVE_COMPLETED** → **SHOP_ACTIVE** (after completion message duration)
3. **SHOP_ACTIVE** → **SHOP_CLOSING** (when player closes shop)  
4. **SHOP_CLOSING** → **WAVE_STARTING** (after post-shop pause)
5. **WAVE_STARTING** → **WAVE_ACTIVE** (after pre-wave pause)

### State-Specific Behaviors

#### WAVE_COMPLETED State
- **Duration**: 2.0 seconds
- **Actions on entry**:
  - Clear all enemies from the map (`enemies.clear()`)
  - Clear all enemy bullets (`bullets.erase()` where `isEnemy == true`)
  - Clear all spawn indicators (`spawnIndicators.clear()`)
  - Distribute bagged materials to player
  - Stop enemy spawning
- **Visual feedback**:
  - Display "WAVE X COMPLETED!" message in center of screen
  - Use large, prominent font (TTF font size 32-36)
  - White text with dark semi-transparent background
- **Game logic**: Pause all gameplay updates except UI



#### SHOP_ACTIVE State
- **Duration**: Until player closes shop
- **Actions on entry**:
  - Call `shop->openShop(wave)`
  - Enable shop input handling
- **Visual feedback**: Standard shop UI
- **Game logic**: Shop handles all input, game logic remains paused

#### SHOP_CLOSING State
- **Duration**: 1.0 seconds  
- **Actions on entry**:
  - Call `shop->closeShop()`
  - Disable shop input handling
- **Visual feedback**:
  - Show "Preparing for next wave..." message
- **Game logic**: Prepare for next wave but don't start it yet

#### WAVE_STARTING State
- **Duration**: 2.0 seconds
- **Actions on entry**: 
  - Reset timeSinceLastSpawn to 0
  - Clear any remaining game entities as safety measure
- **Visual feedback**:
  - Show "WAVE X STARTING..." countdown
  - Display countdown: "3... 2... 1... FIGHT!"
- **Game logic**: Still paused, countdown is purely visual

### Implementation Details

#### New Game Class Members
```cpp
// Add to Game.h
private:
    GameState currentState;
    float stateTimer;
    float stateDuration;
    
    void enterState(GameState newState);
    void updateState(float deltaTime);
    void renderStateUI();
```

#### State Timer Management
- Each state has a specific duration except SHOP_ACTIVE
- `stateTimer` tracks time in current state
- `stateDuration` defines how long to stay in current state
- Automatic transition when `stateTimer >= stateDuration`

#### Enemy Cleanup Strategy
When entering WAVE_COMPLETED state:
1. Call `enemies.clear()` to remove all enemies immediately
2. Filter bullets to remove enemy projectiles:
   ```cpp
   bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
       [](const std::unique_ptr<Bullet>& bullet) {
           return bullet->isEnemyBullet();
       }), bullets.end());
   ```
3. Clear spawn indicators to prevent late spawns
4. Stop all enemy spawning until next wave starts

#### Update Loop Modifications
- Check current state in `Game::update()`
- Only run full game logic during WAVE_ACTIVE state
- During transition states, only update state timer and UI
- Block input appropriately per state

#### UI Rendering
- Add state-specific UI rendering in `Game::renderUI()`
- Use TTF fonts for transition messages when available
- Fallback to bitmap font for compatibility
- Center messages on screen with semi-transparent backgrounds

## Boss Wave Considerations
- Boss waves should follow same transition system
- When boss dies, immediately enter WAVE_COMPLETED state
- Boss death should clear any remaining minions/projectiles
- Same completion message and timing as regular waves

## Configuration Values
```cpp
static const float WAVE_COMPLETED_DURATION = 2.0f;
static const float SHOP_OPENING_DURATION = 1.0f; 
static const float SHOP_CLOSING_DURATION = 1.0f;
static const float WAVE_STARTING_DURATION = 2.0f;
```

## Questions for Clarification

1. **Completion message content**: Should it show just "WAVE X COMPLETED!" or include additional stats (enemies killed, materials gained, time taken)?
Only the wave number and the message "WAVE X COMPLETED!"

2. **Sound effects**: Should we add audio cues for wave completion and transitions? (Sound system not currently implemented)
No, we don't need sound effects for this.

3. **Visual effects**: Any special effects desired for enemy disappearance? (fade out, poof animation, etc.) or just instant removal?
No

4. **Player movement during transitions**: Should player be able to move during transition states or should they be frozen?
Yes

5. **Countdown styling**: For the "3... 2... 1... FIGHT!" countdown, should it be animated (scaling, color changes) or static text?
No need for countdown

6. **Interruption handling**: What should happen if player dies during a transition state? Immediately go to game over or finish the transition first?
Make the player invincible during the transition states.

7. **Skip functionality**: Should players be able to press a key to skip transition delays and get to the shop/next wave faster?
No

8. **Material collection**: Should uncollected experience orbs and materials be auto-collected during wave completion or lost?
Allow the player to collect the materials.

9. **Weapon reloading**: Should all weapons be considered "reloaded" at the start of each wave?
Yes

10. **Screen effects**: Should the screen flash, shake, or have other effects during wave completion to make it more impactful?
No

## Implementation Priority
1. **Phase 1**: Core state machine and enemy cleanup
2. **Phase 2**: Transition messages and timing
3. **Phase 3**: Polish and visual improvements
4. **Phase 4**: Additional features based on answers to questions above

This system will provide much better pacing and player feedback while maintaining the core gameplay loop.
