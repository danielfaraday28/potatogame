#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Vector2.h"

class Bomb {
public:
    Bomb(Vector2 position, float timer, float radius, int damage);
    ~Bomb();
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    bool isExploded() const { return exploded; }
    bool isAlive() const { return alive; }
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getDamage() const { return damage; }
    
private:
    Vector2 position;
    float timer;
    float maxTimer;
    float radius;
    int damage;
    bool exploded;
    bool alive;
    
    // Visual states
    float flashTimer;
    float flashInterval;
    bool showFlash;
    SDL_Texture* bombTexture;
    
    void explode();
    void loadTexture(SDL_Renderer* renderer);
    void renderCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, bool filled);
};