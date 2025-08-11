#include "Player.h"
#include <cmath>
#include <iostream>

Player::Player(float x, float y) 
    : position(x, y), velocity(0, 0), shootDirection(1, 0), 
      radius(20), health(100), shootCooldown(0.15f), timeSinceLastShot(0),
      experience(0), level(1), healthRegenTimer(0) {
    // Initialize health to match max health
    health = stats.maxHealth;
    
    // Start with a basic pistol
    addWeapon(std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_1));
}

void Player::update(float deltaTime) {
    position += velocity * deltaTime;
    
    if (position.x < radius) position.x = radius;
    if (position.x > 1024 - radius) position.x = 1024 - radius;
    if (position.y < radius) position.y = radius;
    if (position.y > 768 - radius) position.y = 768 - radius;
    
    velocity = Vector2(0, 0);
    
    timeSinceLastShot += deltaTime;
    
    // Health regeneration
    if (stats.healthRegen > 0) {
        healthRegenTimer += deltaTime;
        if (healthRegenTimer >= 1.0f) { // Regen every second
            health += (int)stats.healthRegen;
            if (health > stats.maxHealth) health = stats.maxHealth;
            healthRegenTimer = 0.0f;
        }
    }
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
    
    // Draw gun direction line
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    SDL_RenderDrawLine(renderer, centerX, centerY, 
                      centerX + shootDirection.x * 30, 
                      centerY + shootDirection.y * 30);
    
    // Draw pickup range indicator (faint circle)
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 30);
    int pickupR = (int)stats.pickupRange;
    for (int angle = 0; angle < 360; angle += 10) {
        float rad = angle * M_PI / 180.0f;
        int x1 = centerX + cos(rad) * pickupR;
        int y1 = centerY + sin(rad) * pickupR;
        SDL_RenderDrawPoint(renderer, x1, y1);
    }
}

void Player::handleInput(const Uint8* keyState) {
    float currentSpeed = stats.moveSpeed;
    
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP]) {
        velocity.y = -currentSpeed;
    }
    if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN]) {
        velocity.y = currentSpeed;
    }
    if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT]) {
        velocity.x = -currentSpeed;
    }
    if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT]) {
        velocity.x = currentSpeed;
    }
}

void Player::updateShootDirection(const Vector2& mousePosition) {
    Vector2 direction = mousePosition - position;
    float length = direction.length();
    
    // Only update direction if mouse is not exactly on the player
    if (length > 0.1f) {
        shootDirection = direction.normalized();
    }
}

void Player::gainExperience(int exp) {
    experience += exp;
    
    // Check for level up
    while (experience >= getExperienceToNextLevel()) {
        levelUp();
    }
}

void Player::levelUp() {
    level++;
    std::cout << "Level up! Now level " << level << std::endl;
    
    // Brotato-style: +1 Max HP per level
    stats.maxHealth += 1;
    
    // Add weapons at certain levels for testing
    if (level == 3 && weapons.size() < MAX_WEAPONS) {
        addWeapon(std::make_unique<Weapon>(WeaponType::SMG, WeaponTier::TIER_1));
        std::cout << "Got SMG!" << std::endl;
    } else if (level == 5 && weapons.size() < MAX_WEAPONS) {
        addWeapon(std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_2));
        std::cout << "Got Tier 2 Pistol!" << std::endl;
    }
    
    // In Brotato, upgrades are chosen by the player at wave end
    // For now, we'll apply small automatic upgrades
    // TODO: Implement upgrade selection system
    
    // Don't heal on level up in Brotato
}

int Player::getExperienceToNextLevel() const {
    // Brotato's experience formula: (Level + 3) * (Level + 3)
    int nextLevel = level + 1;
    return (nextLevel + 3) * (nextLevel + 3);
}

void Player::takeDamage(int damage) {
    // Apply armor reduction
    int actualDamage = damage - stats.armor;
    if (actualDamage < 1) actualDamage = 1; // Always take at least 1 damage
    
    // Check dodge chance
    if (stats.dodgeChance > 0) {
        float dodgeRoll = (rand() % 100) / 100.0f;
        if (dodgeRoll < stats.dodgeChance / 100.0f) {
            std::cout << "Dodged!" << std::endl;
            return; // Dodged the attack
        }
    }
    
    health -= actualDamage;
    if (health < 0) health = 0;
}

bool Player::canShoot() const {
    return timeSinceLastShot >= (shootCooldown / stats.attackSpeed);
}

void Player::shoot() {
    timeSinceLastShot = 0;
}

void Player::addWeapon(std::unique_ptr<Weapon> weapon) {
    if (weapons.size() < MAX_WEAPONS) {
        weapons.push_back(std::move(weapon));
    }
}

void Player::updateWeapons(float deltaTime, std::vector<std::unique_ptr<Bullet>>& bullets) {
    for (auto& weapon : weapons) {
        weapon->update(deltaTime, position, shootDirection, bullets, *this);
    }
}

void Player::renderWeapons(SDL_Renderer* renderer) {
    for (auto& weapon : weapons) {
        weapon->render(renderer, position);
    }
}