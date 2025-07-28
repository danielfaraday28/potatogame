#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

class Bullet {
public:
    Bullet(Vector2 pos, Vector2 dir, float speed = 400.0f);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    bool isAlive() const { return alive; }
    void destroy() { alive = false; }
    
private:
    Vector2 position;
    Vector2 direction;
    float speed;
    float radius;
    bool alive;
};