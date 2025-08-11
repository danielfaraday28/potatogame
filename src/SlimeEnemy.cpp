#include "SlimeEnemy.h"
#include "Bullet.h"
#include <SDL2/SDL_image.h>
#include <cmath>
#include <iostream>

SlimeEnemy::SlimeEnemy(Vector2 pos, SDL_Renderer* renderer)
    : Enemy(pos, renderer), fireCooldown(2.0f), timeSinceLastShot(0.0f), slimeTexture(nullptr) {
    // Tweak base stats to feel different
    speed = 60.0f;
    radius = 26.0f;
    damage = 8;
    loadSprites(renderer);
}

void SlimeEnemy::loadSprites(SDL_Renderer* renderer) {
    // Use provided slime icon if available
    SDL_Surface* surface = IMG_Load("assets/enemies/slime.png");
    if (!surface) {
        std::cout << "Failed to load slime.png: " << IMG_GetError() << std::endl;
        slimeTexture = nullptr;
        return;
    }
    slimeTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!slimeTexture) {
        std::cout << "Failed to create slime texture: " << SDL_GetError() << std::endl;
    }
}

void SlimeEnemy::tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    timeSinceLastShot += deltaTime;
    if (timeSinceLastShot < fireCooldown) return;
    timeSinceLastShot = 0.0f;

    // Simple straight line shooting towards player
    Vector2 toPlayer = playerPos - position;
    if (toPlayer.length() < 1.0f) return;

    // Shoot straight at the player with normal bullet physics
    Vector2 dir = toPlayer.normalized();
    bullets.push_back(std::make_unique<Bullet>(position, dir, /*damage*/ 10, /*range*/ 600.0f, /*speed*/ 320.0f, BulletType::PISTOL, /*enemyOwned*/ true));
}

void SlimeEnemy::update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // Slow homing movement
    Vector2 direction = (playerPos - position).normalized();
    velocity = direction * speed;
    position += velocity * deltaTime;

    // Fire lob occasionally
    tryFireAtPlayer(deltaTime, playerPos, bullets);

    // Call base animation handling by duplicating minimal logic (state timers are protected in base)
    animationTimer += deltaTime;
    if (state == EnemyState::HIT) {
        hitTimer += deltaTime;
        if (hitTimer > 0.2f) {
            state = EnemyState::IDLE;
            hitTimer = 0.0f;
        }
    }
    if (state == EnemyState::IDLE && animationTimer > 0.5f) {
        currentFrame = (currentFrame == 0) ? 1 : 0;
        animationTimer = 0.0f;
    }
}

void SlimeEnemy::render(SDL_Renderer* renderer) {
    if (!alive) return;
    if (slimeTexture) {
        int w, h;
        SDL_QueryTexture(slimeTexture, nullptr, nullptr, &w, &h);
        float scale = 0.9f;
        int sw = (int)(w * scale);
        int sh = (int)(h * scale);
        SDL_Rect dst{ (int)(position.x - sw/2), (int)(position.y - sh/2), sw, sh };
        SDL_RenderCopy(renderer, slimeTexture, nullptr, &dst);
    } else {
        // fallback circle in green
        SDL_SetRenderDrawColor(renderer, 80, 200, 80, 255);
        int cx = (int)position.x;
        int cy = (int)position.y;
        int r = (int)radius;
        for (int x = -r; x <= r; ++x) {
            for (int y = -r; y <= r; ++y) {
                if (x*x + y*y <= r*r) SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

std::unique_ptr<Enemy> CreateSlimeEnemy(const Vector2& pos, SDL_Renderer* renderer) {
    return std::make_unique<SlimeEnemy>(pos, renderer);
}

