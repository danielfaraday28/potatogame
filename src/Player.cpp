#include "Player.h"
#include <cmath>

Player::Player(float x, float y) 
    : position(x, y), velocity(0, 0), shootDirection(1, 0), 
      radius(20), speed(200), health(100), maxHealth(100),
      shootCooldown(0.15f), timeSinceLastShot(0) {
}

void Player::update(float deltaTime) {
    position += velocity * deltaTime;
    
    if (position.x < radius) position.x = radius;
    if (position.x > 1024 - radius) position.x = 1024 - radius;
    if (position.y < radius) position.y = radius;
    if (position.y > 768 - radius) position.y = 768 - radius;
    
    velocity = Vector2(0, 0);
    
    timeSinceLastShot += deltaTime;
}

void Player::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
    
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
    
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    SDL_RenderDrawLine(renderer, centerX, centerY, 
                      centerX + shootDirection.x * 30, 
                      centerY + shootDirection.y * 30);
}

void Player::handleInput(const Uint8* keyState) {
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP]) {
        velocity.y = -speed;
    }
    if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN]) {
        velocity.y = speed;
    }
    if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT]) {
        velocity.x = -speed;
    }
    if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT]) {
        velocity.x = speed;
    }
    
    // Removed IJKL keyboard aiming - now using mouse aiming instead
}

void Player::updateShootDirection(const Vector2& mousePosition) {
    Vector2 direction = mousePosition - position;
    float length = direction.length();
    
    // Only update direction if mouse is not exactly on the player
    if (length > 0.1f) {
        shootDirection = direction.normalized();
    }
}

void Player::takeDamage(int damage) {
    health -= damage;
    if (health < 0) health = 0;
}

bool Player::canShoot() const {
    return timeSinceLastShot >= shootCooldown;
}

void Player::shoot() {
    timeSinceLastShot = 0;
}