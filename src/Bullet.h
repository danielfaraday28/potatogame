#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

enum class BulletType {
    PISTOL,
    SMG
};

class Bullet {
public:
    Bullet(Vector2 pos, Vector2 dir, int damage = 10, float range = 200.0f, float speed = 400.0f, BulletType type = BulletType::PISTOL);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getDamage() const { return damage; }
    bool isAlive() const { return alive; }
    void destroy() { alive = false; }
    
private:
    Vector2 position;
    Vector2 startPosition;
    Vector2 direction;
    float speed;
    float radius;
    float maxRange;
    int damage;
    bool alive;
    BulletType bulletType;
};