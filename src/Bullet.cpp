#include "Bullet.h"

Bullet::Bullet(Vector2 pos, Vector2 dir, int dmg, float range, float speed, BulletType type) 
    : position(pos), startPosition(pos), direction(dir.normalized()), 
      speed(speed), radius(3), maxRange(range), damage(dmg), alive(true), bulletType(type) {
}

void Bullet::update(float deltaTime) {
    position += direction * speed * deltaTime;
    
    // Check if bullet is out of bounds
    if (position.x < 0 || position.x > 1024 || position.y < 0 || position.y > 768) {
        alive = false;
    }
    
    // Check if bullet has exceeded its range
    float distanceTraveled = startPosition.distance(position);
    if (distanceTraveled > maxRange) {
        alive = false;
    }
}

void Bullet::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Different colors for different weapon types
    switch (bulletType) {
        case BulletType::PISTOL:
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for pistol
            break;
        case BulletType::SMG:
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // Red for SMG
            break;
    }
    
    int centerX = (int)position.x;
    int centerY = (int)position.y;
    int r = (int)radius;
    
    for (int x = -r; x <= r; x++) {
        for (int y = -r; y <= r; y++) {
            if (x*x + y*y <= r*r) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}