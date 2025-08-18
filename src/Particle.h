#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

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
    float alpha;                 // For fade-out effect (0-255)
};
