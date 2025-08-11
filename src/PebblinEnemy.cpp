#include "PebblinEnemy.h"
#include "Bullet.h"
#include <SDL2/SDL_image.h>
#include <cmath>
#include <iostream>

PebblinEnemy::PebblinEnemy(Vector2 pos, SDL_Renderer* renderer)
    : Enemy(pos, renderer), fireCooldown(3.5f), timeSinceLastShot(0.0f), pebblinTexture(nullptr) {
    // Heavy, tanky stats - slower but tougher than slimes
    speed = 35.0f;          // Slower than slime (60.0f)
    radius = 30.0f;         // Slightly bigger
    damage = 12;            // More damage
    health = 40;            // More HP than base enemy
    loadSprites(renderer);
}

void PebblinEnemy::loadSprites(SDL_Renderer* renderer) {
    // Load pebblin sprite
    SDL_Surface* surface = IMG_Load("assets/enemies/pebblin.png");
    if (!surface) {
        std::cout << "Failed to load pebblin.png: " << IMG_GetError() << std::endl;
        pebblinTexture = nullptr;
        return;
    }
    pebblinTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!pebblinTexture) {
        std::cout << "Failed to create pebblin texture: " << SDL_GetError() << std::endl;
    }
}

void PebblinEnemy::tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    timeSinceLastShot += deltaTime;
    if (timeSinceLastShot < fireCooldown) return;
    timeSinceLastShot = 0.0f;

    // Three-shot spread - center shot + two angled shots
    Vector2 toPlayer = playerPos - position;
    if (toPlayer.length() < 1.0f) return;

    Vector2 centerDir = toPlayer.normalized();
    float spreadAngle = 0.3f; // ~17 degrees spread
    
    // Center shot
    bullets.push_back(std::make_unique<Bullet>(position, centerDir, /*damage*/ 8, /*range*/ 500.0f, /*speed*/ 280.0f, BulletType::PISTOL, /*enemyOwned*/ true));
    
    // Left angled shot
    Vector2 leftDir = Vector2(
        centerDir.x * cos(-spreadAngle) - centerDir.y * sin(-spreadAngle),
        centerDir.x * sin(-spreadAngle) + centerDir.y * cos(-spreadAngle)
    );
    bullets.push_back(std::make_unique<Bullet>(position, leftDir, /*damage*/ 8, /*range*/ 500.0f, /*speed*/ 280.0f, BulletType::PISTOL, /*enemyOwned*/ true));
    
    // Right angled shot
    Vector2 rightDir = Vector2(
        centerDir.x * cos(spreadAngle) - centerDir.y * sin(spreadAngle),
        centerDir.x * sin(spreadAngle) + centerDir.y * cos(spreadAngle)
    );
    bullets.push_back(std::make_unique<Bullet>(position, rightDir, /*damage*/ 8, /*range*/ 500.0f, /*speed*/ 280.0f, BulletType::PISTOL, /*enemyOwned*/ true));
}

void PebblinEnemy::update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // Heavy, slow movement toward player
    Vector2 direction = (playerPos - position).normalized();
    velocity = direction * speed;
    position += velocity * deltaTime;

    // Fire rock spread occasionally
    tryFireAtPlayer(deltaTime, playerPos, bullets);

    // Handle animation states
    animationTimer += deltaTime;
    if (state == EnemyState::HIT) {
        hitTimer += deltaTime;
        if (hitTimer > 0.2f) {
            state = EnemyState::IDLE;
            hitTimer = 0.0f;
        }
    }
    if (state == EnemyState::IDLE && animationTimer > 0.7f) { // Slower animation than slime
        currentFrame = (currentFrame == 0) ? 1 : 0;
        animationTimer = 0.0f;
    }
}

void PebblinEnemy::render(SDL_Renderer* renderer) {
    if (!alive) return;
    if (pebblinTexture) {
        int w, h;
        SDL_QueryTexture(pebblinTexture, nullptr, nullptr, &w, &h);
        float scale = 1.0f; // Slightly bigger than slime
        int sw = (int)(w * scale);
        int sh = (int)(h * scale);
        SDL_Rect dst{ (int)(position.x - sw/2), (int)(position.y - sh/2), sw, sh };
        SDL_RenderCopy(renderer, pebblinTexture, nullptr, &dst);
    } else {
        // Fallback circle in gray/brown for rock
        SDL_SetRenderDrawColor(renderer, 120, 100, 80, 255);
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

std::unique_ptr<Enemy> CreatePebblinEnemy(const Vector2& pos, SDL_Renderer* renderer) {
    return std::make_unique<PebblinEnemy>(pos, renderer);
}