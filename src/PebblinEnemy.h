#pragma once
#include "Enemy.h"
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class Bullet;

class PebblinEnemy : public Enemy {
public:
    PebblinEnemy(Vector2 pos, SDL_Renderer* renderer);
    ~PebblinEnemy() override = default;
    
    void update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void render(SDL_Renderer* renderer) override;
    EnemyType getEnemyType() const override { return EnemyType::PEBBLIN; }
    
private:
    void loadSprites(SDL_Renderer* renderer);
    void tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    
    float fireCooldown;
    float timeSinceLastShot;
    SDL_Texture* pebblinTexture;
};

// Factory function
std::unique_ptr<Enemy> CreatePebblinEnemy(const Vector2& pos, SDL_Renderer* renderer);