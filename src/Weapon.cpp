#include "Weapon.h"
#include "Enemy.h"
#include "Player.h"
#include <cmath>
#include <algorithm>
#include <random>

Weapon::Weapon(WeaponType weaponType, WeaponTier weaponTier) 
    : type(weaponType), tier(weaponTier), timeSinceLastShot(0.0f), 
      muzzleFlashTimer(0.0f), lastShotDirection(1, 0) {
    
    // Initialize stats based on weapon type and tier
    switch (type) {
        case WeaponType::PISTOL:
            initializePistolStats();
            break;
        case WeaponType::SMG:
            initializeSMGStats();
            break;
    }
}

void Weapon::initializePistolStats() {
    // Pistol stats based on tier
    switch (tier) {
        case WeaponTier::TIER_1:
            stats.baseDamage = 12;
            stats.attackSpeed = 1.2f;
            stats.critChance = 0.05f;
            break;
        case WeaponTier::TIER_2:
            stats.baseDamage = 20;
            stats.attackSpeed = 1.12f;
            stats.critChance = 0.10f;
            break;
        case WeaponTier::TIER_3:
            stats.baseDamage = 30;
            stats.attackSpeed = 1.03f;
            stats.critChance = 0.15f;
            break;
        case WeaponTier::TIER_4:
            stats.baseDamage = 50;
            stats.attackSpeed = 0.87f;
            stats.critChance = 0.20f;
            break;
    }
    
    stats.range = 400;
    stats.critMultiplier = 2.0f;
    stats.knockback = 15;
    stats.rangedDamageScaling = 1.0f;
}

void Weapon::initializeSMGStats() {
    // SMG stats based on tier
    switch (tier) {
        case WeaponTier::TIER_1:
            stats.baseDamage = 3;
            stats.attackSpeed = 0.17f;
            break;
        case WeaponTier::TIER_2:
            stats.baseDamage = 4;
            stats.attackSpeed = 0.16f;
            break;
        case WeaponTier::TIER_3:
            stats.baseDamage = 6;
            stats.attackSpeed = 0.155f;
            break;
        case WeaponTier::TIER_4:
            stats.baseDamage = 8;
            stats.attackSpeed = 0.15f;
            break;
    }
    
    stats.range = 400;
    stats.critChance = 0.01f;
    stats.critMultiplier = 1.5f;
    stats.knockback = 0;
    stats.rangedDamageScaling = 1.0f;
}

void Weapon::update(float deltaTime, const Vector2& playerPos, 
                   const Vector2& aimDirection,
                   std::vector<std::unique_ptr<Bullet>>& bullets,
                   const Player& player) {
    
    timeSinceLastShot += deltaTime;
    muzzleFlashTimer = std::max(0.0f, muzzleFlashTimer - deltaTime);
    
    // Fire in the direction player is aiming if ready
    if (canFire()) {
        fire(playerPos, aimDirection, bullets, player);
        timeSinceLastShot = 0.0f;
        muzzleFlashTimer = 0.1f; // Show muzzle flash for 0.1 seconds
        lastShotDirection = aimDirection;
    }
}

void Weapon::render(SDL_Renderer* renderer, const Vector2& playerPos) {
    // Draw weapon indicator (simple line pointing in last shot direction)
    if (muzzleFlashTimer > 0) {
        // Muzzle flash effect
        SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
        
        Vector2 muzzlePos = playerPos + lastShotDirection * 25;
        SDL_RenderDrawLine(renderer, 
                          (int)playerPos.x, (int)playerPos.y,
                          (int)muzzlePos.x, (int)muzzlePos.y);
        
        // Flash circle
        int flashRadius = 8;
        for (int x = -flashRadius; x <= flashRadius; x++) {
            for (int y = -flashRadius; y <= flashRadius; y++) {
                if (x*x + y*y <= flashRadius*flashRadius) {
                    SDL_RenderDrawPoint(renderer, (int)muzzlePos.x + x, (int)muzzlePos.y + y);
                }
            }
        }
    } else {
        // Normal weapon indicator
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        Vector2 weaponEnd = playerPos + lastShotDirection * 15;
        SDL_RenderDrawLine(renderer, 
                          (int)playerPos.x, (int)playerPos.y,
                          (int)weaponEnd.x, (int)weaponEnd.y);
    }
}

void Weapon::fire(const Vector2& playerPos, const Vector2& direction, 
                 std::vector<std::unique_ptr<Bullet>>& bullets,
                 const Player& player) {
    
    Vector2 fireDirection = direction;
    
    // Add inaccuracy for SMG
    if (type == WeaponType::SMG) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> inaccuracy(-0.2f, 0.2f);
        
        float angle = atan2(fireDirection.y, fireDirection.x);
        angle += inaccuracy(gen);
        fireDirection = Vector2(cos(angle), sin(angle));
    }
    
    int finalDamage = calculateDamage(player);
    
    // Check for critical hit
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> critRoll(0.0f, 1.0f);
    
    if (critRoll(gen) < stats.critChance) {
        finalDamage = (int)(finalDamage * stats.critMultiplier);
    }
    
    // Create bullet with appropriate type
    BulletType bulletType = (type == WeaponType::SMG) ? BulletType::SMG : BulletType::PISTOL;
    bullets.push_back(std::make_unique<Bullet>(playerPos, fireDirection, finalDamage, stats.range, 400.0f, bulletType));
    
    // Special weapon effects
    if (type == WeaponType::PISTOL) {
        // Pistol pierces 1 enemy with -50% damage
        // This will be handled in the Bullet class collision detection
    }
}


int Weapon::calculateDamage(const Player& player) const {
    float totalDamage = stats.baseDamage;
    
    // Apply scaling from player stats
    if (stats.rangedDamageScaling > 0) {
        // For now, we'll use player's base damage as ranged damage
        // In a full implementation, Player would have separate ranged damage stat
        totalDamage += player.getStats().damage * stats.rangedDamageScaling;
    }
    
    return (int)totalDamage;
}