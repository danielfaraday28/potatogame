#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

class Material {
public:
    Material(Vector2 pos, int materialValue = 1, int expValue = 1);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getMaterialValue() const { return materialValue; }
    int getExperienceValue() const { return experienceValue; }
    bool isAlive() const { return alive; }
    void collect() { alive = false; }
    
private:
    Vector2 position;
    float radius;
    int materialValue;
    int experienceValue;
    bool alive;
    float lifetime;
    float maxLifetime;
    float bobOffset;
    float bobSpeed;
};