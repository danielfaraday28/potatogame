#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

class ExperienceOrb {
public:
    ExperienceOrb(Vector2 pos, int expValue = 1);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getExperienceValue() const { return experienceValue; }
    bool isAlive() const { return alive; }
    void collect() { alive = false; }
    
private:
    Vector2 position;
    float radius;
    int experienceValue;
    bool alive;
    float lifetime;
    float maxLifetime;
    float bobOffset;
    float bobSpeed;
}; 