#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

struct PlayerStats {
    // Brotato-like stats
    int maxHealth;
    float moveSpeed;
    float pickupRange;
    float attackSpeed;
    int damage;
    float range;
    int armor;
    float healthRegen;
    float dodgeChance;
    int luck;
    int materials; // Add materials currency like Brotato
    
    PlayerStats() : maxHealth(100), moveSpeed(200), pickupRange(50), 
                   attackSpeed(1.0f), damage(10), range(200), 
                   armor(0), healthRegen(0), dodgeChance(0), luck(0), materials(0) {}
};

class Player {
public:
    Player(float x, float y);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void handleInput(const Uint8* keyState);
    void updateShootDirection(const Vector2& mousePosition);
    void gainExperience(int exp);
    void levelUp();
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getHealth() const { return health; }
    void takeDamage(int damage);
    
    bool canShoot() const;
    void shoot();
    Vector2 getShootDirection() const { return shootDirection; }
    
    // Stats and progression
    PlayerStats& getStats() { return stats; }
    const PlayerStats& getStats() const { return stats; }
    int getExperience() const { return experience; }
    int getLevel() const { return level; }
    int getExperienceToNextLevel() const;
    void gainMaterials(int amount) { stats.materials += amount; }
    
private:
    Vector2 position;
    Vector2 velocity;
    Vector2 shootDirection;
    float radius;
    int health;
    float shootCooldown;
    float timeSinceLastShot;
    
    // Progression system
    PlayerStats stats;
    int experience;
    int level;
    float healthRegenTimer;
};