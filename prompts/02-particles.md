# Particle System Design Document

## Overview

This document describes the design for a particle effect system to be integrated into the Brotato MVP game. The system will provide visual effects for combat, explosions, enemy deaths, and other game events.

## Requirements

- **Single Emitter Type**: Circle emitter that sends particles from its center
- **Colorable Particles**: Support for tinting the white star.png texture
- **Single Global Array**: One collection of particles managed by the Game class
- **No Cleanup**: Particles persist for the entire game session (simplified memory management)
- **Two-Phase Lifecycle**: Normal flight phase followed by disappearing phase

## Architecture

### Core Classes

#### 1. `Particle` Class
**Purpose**: Represents individual particle instances
**Location**: `src/Particle.h/cpp`

```cpp
class Particle {
public:
    Particle(Vector2 position, Vector2 velocity, float normalDuration, 
             SDL_Color color, float scale = 1.0f);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer, SDL_Texture* starTexture);
    
    bool isAlive() const { return alive; }
    Vector2 getPosition() const { return position; }
    
private:
    Vector2 position;
    Vector2 velocity;
    SDL_Color color;
    float scale;
    
    // Lifecycle management
    bool alive;
    float lifetime;
    float normalDuration;        // Base duration for normal phase
    float disappearDuration;     // 20% of normal phase
    float totalDuration;         // normalDuration + disappearDuration
    
    enum class Phase { NORMAL, DISAPPEARING } phase;
    float alpha;                 // For fade-out effect
};
```

#### 2. Simplified Particle Creation
**Purpose**: Direct particle creation functions - no emitter class needed
**Location**: `src/Game.h/cpp` (modifications)

Since emitters are instant bursts only, we'll use direct creation functions instead of a separate emitter class.

```cpp
// In Game.h - Add to private members:
std::vector<std::unique_ptr<Particle>> particles;
SDL_Texture* starTexture;

// Public methods for creating effects:
void createParticleBurst(Vector2 position, int particleCount, float particleSpeed,
                        float normalDuration, SDL_Color color, float scale = 1.0f);
void createExplosionEffect(Vector2 position, SDL_Color color = {255, 255, 255, 255});
void createDeathEffect(Vector2 position, SDL_Color color = {255, 0, 0, 255});
void createImpactEffect(Vector2 position, SDL_Color color = {255, 255, 0, 255});
```

## Particle Lifecycle

### Phase 1: Normal
- **Duration**: Configurable per effect type (parameter + 10% randomization)
- **Behavior**: Particle flies in straight line with constant velocity
- **Appearance**: Full opacity, normal color

### Phase 2: Disappearing  
- **Duration**: 20% of normal phase duration
- **Behavior**: Continues flying with same velocity
- **Appearance**: Alpha fades from 255 to 0 linearly

### Timing Example
```
Normal Duration: 2.0s (with ±10% randomization = 1.8s - 2.2s)
Disappear Duration: 0.4s (20% of 2.0s)
Total Lifetime: 2.4s (approximately)
```

## Rendering

### Additive Blending
```cpp
// In particle render method:
SDL_SetTextureBlendMode(starTexture, SDL_BLENDMODE_ADD);
SDL_SetTextureColorMod(starTexture, color.r, color.g, color.b);
SDL_SetTextureAlphaMod(starTexture, alpha);
```

### Color Tinting
- Star texture is white (255, 255, 255)
- Color modulation allows full RGB spectrum
- Alpha modulation handles fade-out effect

## Integration Points

### 1. Game Loop Integration
```cpp
// In Game::update()
void Game::update(float deltaTime) {
    // ... existing updates ...
    
    updateParticles(deltaTime);
}

// In Game::render()
void Game::render() {
    // ... existing rendering ...
    
    renderParticles();  // Render after background, before UI
}
```

### 2. Effect API Usage Examples (For Future Integration)
```cpp
// Example usage patterns (not automatically implemented):
// Enemy Death: createDeathEffect(enemy->getPosition(), {255, 0, 0, 255})
// Weapon Hits: createImpactEffect(bullet->getPosition(), {255, 255, 0, 255})
// Bomb Explosions: createExplosionEffect(bomb->getPosition(), {255, 100, 0, 255})
// Level Up: createParticleBurst(player->getPosition(), 30, 150.0f, 2.0f, {0, 255, 0, 255})
```

### 3. Asset Loading
```cpp
// In Game::init()
starTexture = IMG_LoadTexture(renderer, "assets/particles/star.png");
if (!starTexture) {
    std::cout << "Failed to load star texture: " << IMG_GetError() << std::endl;
    // Continue without particles (graceful degradation)
}
```

## Performance Considerations

### Memory Management
- Use `std::unique_ptr` for automatic cleanup
- Particles never removed (as per requirement)
- Emitters can be removed when no longer needed

### Rendering Optimization
- Batch particles by color for reduced state changes
- Use SDL_RenderCopyEx for rotation if needed later
- Consider particle pooling if performance issues arise

### Update Optimization
- Early exit for inactive emitters
- Vector pre-allocation to avoid reallocations
- Efficient removal patterns using iterators

## Configuration

### Default Effect Presets
```cpp
// Explosion effect
constexpr float EXPLOSION_EMISSION_RATE = 50.0f;  // particles/second
constexpr float EXPLOSION_PARTICLE_SPEED = 200.0f; // pixels/second
constexpr float EXPLOSION_DURATION = 1.5f;         // seconds
constexpr SDL_Color EXPLOSION_COLOR = {255, 100, 0, 255}; // Orange

// Death effect  
constexpr float DEATH_EMISSION_RATE = 20.0f;
constexpr float DEATH_PARTICLE_SPEED = 150.0f;
constexpr float DEATH_DURATION = 1.0f;
constexpr SDL_Color DEATH_COLOR = {255, 0, 0, 255}; // Red

// Impact effect
constexpr float IMPACT_EMISSION_RATE = 30.0f;
constexpr float IMPACT_PARTICLE_SPEED = 100.0f;
constexpr float IMPACT_DURATION = 0.5f;
constexpr SDL_Color IMPACT_COLOR = {255, 255, 0, 255}; // Yellow
```

## Future Extensions

### Potential Enhancements (Not in Scope)
- Multiple particle textures
- Physics-based particle movement
- Particle collision with environment
- Emitter shapes (line, rectangle, etc.)
- Particle scaling over lifetime
- Rotation animations

## Implementation Plan

1. **Create Particle class** - Basic particle with two-phase lifecycle
2. **Create ParticleEmitter class** - Circle emission pattern
3. **Integrate into Game class** - Add collections and update/render loops
4. **Load star texture** - Asset loading with error handling
5. **Add effect creation methods** - Public API for triggering effects
6. **Hook up to game events** - Enemy death, explosions, etc.
7. **Test and tune** - Adjust parameters for visual appeal

## Design Decisions (Based on Requirements)

1. **Emitter Type**: Instant burst emitters only - no continuous emission over time
2. **Emission Pattern**: Random 360° directions from center point
3. **Particle Limits**: No maximum count limits - particles persist indefinitely
4. **Effect Integration**: API provided for manual triggering only - no automatic game event integration
5. **Implementation Scope**: Particle system infrastructure only - no actual effect instances added to game events
