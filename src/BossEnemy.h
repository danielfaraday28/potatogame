#pragma once
#include "Enemy.h"
#include <random>
#include <vector>
#include <queue>

class Bullet;

enum class BossPhase {
    PHASE1,
    PHASE2,
    ENRAGE
};

enum class BossPattern {
    RADIAL_SHOTS,
    DASH_TO_PLAYER,
    SUMMON_ADDS,
    SPIRAL_SHOTS,
    BURST_AIM
};

struct BossConfig {
    const char* name;
    float scale = 3.0f;     // Увеличим размер
    float hpMul = 5000.f;  // В 100 раз больше изначального HP
    float dmgMul = 4.0f;   // Больше урона
    float speedMul = 0.7f; // Чуть медленнее для баланса
    std::vector<std::pair<BossPattern, float>> weightsP1;
    std::vector<std::pair<BossPattern, float>> weightsP2;
    std::vector<std::pair<BossPattern, float>> weightsEnrage;
    uint32_t seed;
};

class BossEnemy : public Enemy {
public:
    BossEnemy(const Vector2& spawnPos, SDL_Renderer* renderer, const BossConfig& config);
    void update(float dt, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void render(SDL_Renderer* r) override;
    void takeDamage(int amount);
    bool isDead() const { return health <= 0; }
    const char* getName() const { return config.name; }
    float getHealthPercent() const { return maxHealthValue > 0 ? static_cast<float>(health) / static_cast<float>(maxHealthValue) : 0.0f; }

private:
    BossConfig config;
    BossPhase phase = BossPhase::PHASE1;
    float phaseTimer = 0.f;
    float attackCooldown = 0.f;
    std::mt19937 rng;
    std::queue<BossPattern> lastPatterns;
    static constexpr size_t MAX_PATTERN_HISTORY = 3;
    int maxHealthValue = 0;
    Vector2 lastKnownPlayerPos;

    void updatePhase();
    BossPattern nextPattern();
    void performPattern(float dt, std::vector<std::unique_ptr<Bullet>>& bullets);
    void patternRadialShots(std::vector<std::unique_ptr<Bullet>>& bullets);
    void patternDashToPlayer();
    void patternSummonAdds();
    void patternSpiralShots(std::vector<std::unique_ptr<Bullet>>& bullets);
    void patternBurstAim(std::vector<std::unique_ptr<Bullet>>& bullets);
    void telegraph(const Vector2& pos, float radius, float duration);
    
    std::vector<std::pair<BossPattern, float>>& getCurrentWeights();
    bool tooManyRepeats(BossPattern pattern) const;
};
