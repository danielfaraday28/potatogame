#include "BossEnemy.h"
#include "Game.h"
#include <algorithm>
#include <SDL2/SDL_image.h>
#include <cmath>

BossEnemy::BossEnemy(const Vector2& spawnPos, SDL_Renderer* renderer, const BossConfig& cfg)
    : Enemy(spawnPos, renderer)
    , config(cfg)
    , rng(cfg.seed)
{
    // Загружаем текстуру в зависимости от имени босса
    const char* texturePath = "assets/enemies/mortorhead.png";
    if (std::string(cfg.name) == "MEGA SLIME") {
        texturePath = "assets/enemies/slime.png";
    } else if (std::string(cfg.name) == "DARK PEBBLIN") {
        texturePath = "assets/enemies/pebblin.png";
    }
    
    SDL_Surface* surface = IMG_Load(texturePath);
    if (surface) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        // Используем одну текстуру для всех состояний, чтобы базовый рендер работал
        idleFrame1 = tex;
        idleFrame2 = tex;
        hitFrame = tex;
    }

    // Применяем множители из конфига и масштабируем визуальные размеры
    health = static_cast<int>(health * config.hpMul);
    maxHealthValue = health;
    damage = static_cast<int>(damage * config.dmgMul);
    speed = speed * config.speedMul;
    spriteWidth = static_cast<int>(64 * config.scale);
    spriteHeight = static_cast<int>(64 * config.scale);
    radius = static_cast<float>(spriteWidth / 2);
}

void BossEnemy::update(float dt, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    if (!isAlive()) return;
    
    // Сохраняем последнюю известную позицию игрока
    lastKnownPlayerPos = playerPos;
    
    // Обновляем базовую логику движения только если не в рывке
    if (velocity.length() < speed * 4.0f) { // Увеличили порог для лучшего определения рывка
        Enemy::update(dt, playerPos, bullets);
    } else {
        // В рывке - просто обновляем позицию
        position = position + velocity * dt;
        
        // Замедляем рывок со временем (медленнее для более длинных прыжков)
        velocity = velocity * 0.97f;
    }
    
    phaseTimer += dt;
    updatePhase();
    
    // Обработка атак
    if (attackCooldown > 0) {
        attackCooldown -= dt;
    } else {
        // Выбираем и выполняем паттерн атаки
        BossPattern pattern = nextPattern();
        
        switch (pattern) {
            case BossPattern::RADIAL_SHOTS:
                patternRadialShots(bullets);
                attackCooldown = 1.5f;
                break;
            case BossPattern::DASH_TO_PLAYER:
                patternDashToPlayer();
                attackCooldown = 2.5f;
                break;
            case BossPattern::SUMMON_ADDS:
                patternSummonAdds();
                attackCooldown = 5.0f;
                break;
            case BossPattern::SPIRAL_SHOTS:
                patternSpiralShots(bullets);
                attackCooldown = 1.8f;
                break;
            case BossPattern::BURST_AIM:
                patternBurstAim(bullets);
                attackCooldown = 1.2f;
                break;
        }
        
        // Phase modifiers (lighter)
        if (phase == BossPhase::PHASE2) {
            attackCooldown *= 0.9f;
        }
        else if (phase == BossPhase::ENRAGE) {
            attackCooldown *= 0.8f;
        }
    }
}

void BossEnemy::render(SDL_Renderer* r) {
    if (!isAlive()) return;
    Enemy::render(r);
}

void BossEnemy::updatePhase() {
    float healthPercent = getHealthPercent();
    
    // Смена фаз по HP
    if (phase == BossPhase::PHASE1 && healthPercent < 0.7f) {
        phase = BossPhase::PHASE2;
    }
    else if (phase == BossPhase::PHASE2 && healthPercent < 0.3f) {
        phase = BossPhase::ENRAGE;
    }
}

BossPattern BossEnemy::nextPattern() {
    auto& weights = getCurrentWeights();
    
    // Фильтруем паттерны с повторами
    std::vector<std::pair<BossPattern, float>> available;
    for (const auto& [pattern, weight] : weights) {
        if (!tooManyRepeats(pattern)) {
            available.push_back({pattern, weight});
        }
    }
    
    // Если все в повторах, используем все паттерны
    if (available.empty()) {
        available = weights;
    }
    
    // Выбираем взвешенный случайный паттерн
    float totalWeight = 0;
    for (const auto& [_, weight] : available) {
        totalWeight += weight;
    }
    
    std::uniform_real_distribution<float> dist(0, totalWeight);
    float roll = dist(rng);
    
    for (const auto& [pattern, weight] : available) {
        roll -= weight;
        if (roll <= 0) {
            // Добавляем в историю
            if (lastPatterns.size() >= MAX_PATTERN_HISTORY) {
                lastPatterns.pop();
            }
            lastPatterns.push(pattern);
            return pattern;
        }
    }
    
    return available.back().first;
}

void BossEnemy::performPattern(float dt, std::vector<std::unique_ptr<Bullet>>& bullets) {
    BossPattern pattern = nextPattern();
    
    switch (pattern) {
        case BossPattern::RADIAL_SHOTS:
            patternRadialShots(bullets);
            attackCooldown = 1.5f; // Уменьшили кулдаун для более частой стрельбы
            break;
        case BossPattern::DASH_TO_PLAYER:
            patternDashToPlayer();
            attackCooldown = 2.5f; // Увеличили кулдаун прыжков
            break;
        case BossPattern::SUMMON_ADDS:
            patternSummonAdds();
            attackCooldown = 5.0f;
            break;
        case BossPattern::SPIRAL_SHOTS:
            patternSpiralShots(bullets);
            attackCooldown = 1.8f; // Уменьшили кулдаун спиральной стрельбы
            break;
        case BossPattern::BURST_AIM:
            patternBurstAim(bullets);
            attackCooldown = 1.2f; // Быстрая прицельная стрельба
            break;
    }
    
    // Phase modifiers (lighter)
    if (phase == BossPhase::PHASE2) {
        attackCooldown *= 0.9f;
    }
    else if (phase == BossPhase::ENRAGE) {
        attackCooldown *= 0.8f;
    }
}

std::vector<std::pair<BossPattern, float>>& BossEnemy::getCurrentWeights() {
    switch (phase) {
        case BossPhase::PHASE2:
            return config.weightsP2;
        case BossPhase::ENRAGE:
            return config.weightsEnrage;
        default:
        case BossPhase::PHASE1:
            return config.weightsP1;
    }
}

bool BossEnemy::tooManyRepeats(BossPattern pattern) const {
    if (lastPatterns.size() < 2) return false;
    
    size_t count = 0;
    auto temp = lastPatterns;
    while (!temp.empty()) {
        if (temp.front() == pattern) count++;
        temp.pop();
    }
    
    return count >= 2;
}

void BossEnemy::patternRadialShots(std::vector<std::unique_ptr<Bullet>>& bullets) {
    bool isMegaSlime = std::string(config.name) == "MEGA SLIME";
    bool isDarkPebblin = std::string(config.name) == "DARK PEBBLIN";
    
    // Настройки в зависимости от типа босса
    int baseNumBullets;
    float baseSpeed;
    
    if (isMegaSlime) {
        baseNumBullets = 12;
        baseSpeed = 160.0f;
    } else if (isDarkPebblin) {
        baseNumBullets = 8;    // defensive pattern, fewer bullets
        baseSpeed = 220.0f;
    } else {
        baseNumBullets = 10;
        baseSpeed = 200.0f;
    }

    const int numBullets = baseNumBullets + (phase == BossPhase::PHASE2 ? 3 : (phase == BossPhase::ENRAGE ? 6 : 0));
    const float bulletSpeed = baseSpeed + (phase == BossPhase::PHASE2 ? 30.0f : (phase == BossPhase::ENRAGE ? 60.0f : 0.0f));

    for (int i = 0; i < numBullets; ++i) {
        float angle = (2.0f * 3.14159f * i) / numBullets;
        Vector2 direction(cos(angle), sin(angle));
        Vector2 bulletVelocity = direction * bulletSpeed;
        float bulletRange = 600.0f;
        bullets.push_back(std::make_unique<Bullet>(position, bulletVelocity, damage, bulletRange, bulletSpeed, BulletType::PISTOL, true));
    }
}

void BossEnemy::patternDashToPlayer() {
    // Вычисляем направление к последней позиции игрока
    Vector2 direction = (lastKnownPlayerPos - position).normalized();
    
    // Для MEGA SLIME делаем серию рывков
    if (std::string(config.name) == "MEGA SLIME") {
        // Количество рывков зависит от фазы
        int numDashes = 1 + (phase == BossPhase::PHASE2 ? 1 : (phase == BossPhase::ENRAGE ? 2 : 0));
        
        float dashDistance = 300.0f; // was 400
        Vector2 dashEnd = position;
        for (int i = 0; i < numDashes; i++) {
            dashEnd = dashEnd + direction * dashDistance;
            telegraph(dashEnd, 96.0f, 1.0f);
        }

        float dashSpeed = speed * 8.0f; // was 12x
        if (phase == BossPhase::PHASE2) dashSpeed *= 1.3f;
        if (phase == BossPhase::ENRAGE) dashSpeed *= 1.6f;
        
        velocity = direction * dashSpeed;
    } else {
        // Для других боссов - обычный рывок
        telegraph(lastKnownPlayerPos, 64.0f, 1.0f);
        
        float dashSpeed = speed * 5.0f;  // unchanged or reduce to 4.5f if needed
        if (phase == BossPhase::PHASE2) dashSpeed *= 1.15f;
        if (phase == BossPhase::ENRAGE) dashSpeed *= 1.35f;
        
        velocity = direction * dashSpeed;
    }
}

void BossEnemy::patternSummonAdds() {
    // Количество миньонов зависит от фазы
    int numAdds = 2 + (phase == BossPhase::PHASE2 ? 1 : (phase == BossPhase::ENRAGE ? 2 : 0));
    
    for (int i = 0; i < numAdds; ++i) {
        // Спавним врагов по кругу вокруг босса
        float angle = (2.0f * 3.14159f * i) / numAdds;
        float radius = 150.0f;
        Vector2 spawnPos = position + Vector2(cos(angle) * radius, sin(angle) * radius);
        
        // Здесь можно интегрировать систему индикаторов спавна через Game позже
    }
}

void BossEnemy::patternSpiralShots(std::vector<std::unique_ptr<Bullet>>& bullets) {
    static float spiralAngle = 0.0f;
    bool isMegaSlime = std::string(config.name) == "MEGA SLIME";
    bool isMortorhead = std::string(config.name) == "MORTORHEAD PRIME";
    
    // Настройки в зависимости от типа босса
    int baseSpiralCount;
    float baseSpeed;
    float bulletRange = 750.0f; // Увеличенный радиус полёта для всех боссов
    
    if (isMegaSlime) {
        baseSpiralCount = 3;  // Больше спиралей, медленнее
        baseSpeed = 180.0f;
        bulletRange = 600.0f; // Чуть меньше для слайма (компенсация за количество)
    } else if (isMortorhead) {
        baseSpiralCount = 4;  // Ещё больше спиралей для Mortorhead
        baseSpeed = 300.0f;   // И быстрее
        bulletRange = 900.0f; // Больше для Mortorhead (он специалист по спиралям)
    } else {
        baseSpiralCount = 2;
        baseSpeed = 250.0f;
    }
    
    const int spiralsCount = baseSpiralCount + (phase == BossPhase::PHASE2 ? 1 : (phase == BossPhase::ENRAGE ? 2 : 0));
    const float bulletSpeed = baseSpeed;
    
    // Создаем спиральные лучи
    for (int spiral = 0; spiral < spiralsCount; ++spiral) {
        float angle = spiralAngle + (2.0f * 3.14159f * spiral) / spiralsCount;
        Vector2 direction(cos(angle), sin(angle));
        
        auto bullet = std::make_unique<Bullet>(
            position,
            direction * bulletSpeed,
            damage,
            true
        );
        bullets.push_back(std::move(bullet));
    }
    
    // Увеличиваем угол для следующего кадра
    spiralAngle += 0.2f;
    if (spiralAngle > 2.0f * 3.14159f) {
        spiralAngle -= 2.0f * 3.14159f;
    }
}

void BossEnemy::patternBurstAim(std::vector<std::unique_ptr<Bullet>>& bullets) {
    // Вычисляем направление к последней позиции игрока
    Vector2 direction = (lastKnownPlayerPos - position).normalized();
    
    bool isDarkPebblin = std::string(config.name) == "DARK PEBBLIN";
    
    int baseBurstCount = isDarkPebblin ? 4 : 3;    // was 5 for dark pebblin
    float baseBulletSpeed = isDarkPebblin ? 300.0f : 260.0f;

    int burstCount = baseBurstCount + (phase == BossPhase::PHASE2 ? 1 : (phase == BossPhase::ENRAGE ? 3 : 0));
    float bulletSpeed = baseBulletSpeed;

    for (int i = 0; i < burstCount; ++i) {
        float spread = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;
        float baseAngle = atan2(direction.y, direction.x);
        float angle = baseAngle + spread;
        Vector2 bulletDir(cos(angle), sin(angle));
        Vector2 bulletVelocity = bulletDir * bulletSpeed;
        float bulletRange = 650.0f; // was 750
        bullets.push_back(std::make_unique<Bullet>(position, bulletVelocity, damage, bulletRange, bulletSpeed, BulletType::PISTOL, true));
    }
}

void BossEnemy::telegraph(const Vector2& pos, float radius, float duration) {
    // Заглушка: будущая интеграция телеграфа через Game
}

void BossEnemy::takeDamage(int amount) {
    if (!isAlive()) return;
    health -= amount;
    if (health < 0) health = 0;
    hit();
    if (health == 0) {
        alive = false;
    }
}