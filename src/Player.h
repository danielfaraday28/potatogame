#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

class Player {
public:
    Player(float x, float y);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void handleInput(const Uint8* keyState);
    void updateShootDirection(const Vector2& mousePosition); // Add method for mouse aiming
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getHealth() const { return health; }
    void takeDamage(int damage);
    
    bool canShoot() const;
    void shoot();
    Vector2 getShootDirection() const { return shootDirection; }
    
private:
    Vector2 position;
    Vector2 velocity;
    Vector2 shootDirection;
    float radius;
    float speed;
    int health;
    int maxHealth;
    float shootCooldown;
    float timeSinceLastShot;
};