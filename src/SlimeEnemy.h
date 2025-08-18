#pragma once
#include "Enemy.h"
#include <random>
#include <vector>
#include <memory>

class Bullet;

class SlimeEnemy : public Enemy {
public:
    SlimeEnemy(Vector2 pos, SDL_Renderer* renderer);
    ~SlimeEnemy() override = default;

    void update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void render(SDL_Renderer* renderer) override;
    EnemyType getEnemyType() const override { return EnemyType::SLIME; }

private:
    void loadSprites(SDL_Renderer* renderer);
    void tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);

    float fireCooldown;
    float timeSinceLastShot;
    SDL_Texture* slimeTexture;
};

// Factory helper so Game.cpp can spawn without header include order issues
std::unique_ptr<Enemy> CreateSlimeEnemy(const Vector2& pos, SDL_Renderer* renderer);

