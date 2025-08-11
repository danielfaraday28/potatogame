#include "Shop.h"
#include "Player.h"
#include "Game.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
#include <cstring>

std::string ShopItem::getWeaponName(WeaponType wType, WeaponTier wTier) {
    std::string baseName;
    switch (wType) {
        case WeaponType::PISTOL: baseName = "Pistol"; break;
        case WeaponType::SMG: baseName = "SMG"; break;
    }
    
    std::string tierName;
    switch (wTier) {
        case WeaponTier::TIER_1: tierName = ""; break;
        case WeaponTier::TIER_2: tierName = "Tier 2 "; break;
        case WeaponTier::TIER_3: tierName = "Tier 3 "; break;
        case WeaponTier::TIER_4: tierName = "Tier 4 "; break;
    }
    
    return tierName + baseName;
}

std::string ShopItem::getWeaponDescription(WeaponType wType, WeaponTier wTier) {
    switch (wType) {
        case WeaponType::PISTOL:
            return "Pierces 1 enemy, -50% damage to 2nd";
        case WeaponType::SMG:
            return "Fast fire rate, inaccurate shots";
    }
    return "";
}

Shop::Shop() : active(false), rerollCount(0), currentWave(0), selectedItem(0) {
    for (int i = 0; i < 4; i++) {
        keyPressed[i] = false;
    }
}

void Shop::generateItems(int waveNumber, int playerLuck) {
    items.clear();
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    // Generate 4 random items
    for (int i = 0; i < MAX_SHOP_ITEMS; i++) {
        // Choose weapon type
        std::uniform_int_distribution<int> weaponDist(0, 1);
        WeaponType weaponType = (weaponDist(gen) == 0) ? WeaponType::PISTOL : WeaponType::SMG;
        
        // Choose tier based on wave number (Brotato tier restrictions)
        std::vector<WeaponTier> availableTiers;
        availableTiers.push_back(WeaponTier::TIER_1);
        
        if (waveNumber >= 2) availableTiers.push_back(WeaponTier::TIER_2);
        if (waveNumber >= 4) availableTiers.push_back(WeaponTier::TIER_3);
        if (waveNumber >= 8) availableTiers.push_back(WeaponTier::TIER_4);
        
        std::uniform_int_distribution<int> tierDist(0, availableTiers.size() - 1);
        WeaponTier tier = availableTiers[tierDist(gen)];
        
        int price = calculateItemPrice(weaponType, tier, waveNumber);
        items.emplace_back(weaponType, tier, price);
    }
}

void Shop::openShop(int waveNumber) {
    active = true;
    currentWave = waveNumber;
    rerollCount = 0;
    selectedItem = 0;
    generateItems(waveNumber);
    
    std::cout << "Shop opened after wave " << waveNumber << std::endl;
}

void Shop::closeShop() {
    active = false;
    items.clear();
}

void Shop::render(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    if (!active) return;
    
    // Draw shop background
    SDL_SetRenderDrawColor(renderer, 40, 40, 60, 200);
    SDL_Rect shopBg = {100, 100, windowWidth - 200, windowHeight - 200};
    SDL_RenderFillRect(renderer, &shopBg);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &shopBg);
    
    // Shop title
    renderText(renderer, "SHOP - WAVE ", 150, 120, 3);
    renderNumber(renderer, currentWave, 300, 120, 3);
    
    // Instructions
    renderText(renderer, "1-4: Buy Item  L: Lock  R: Reroll  ESC: Close", 150, 160, 1);
    
    // Render items
    int itemWidth = 180;
    int itemHeight = 200;
    int startX = 150;
    int startY = 200;
    
    for (int i = 0; i < items.size(); i++) {
        int x = startX + i * (itemWidth + 20);
        bool selected = (i == selectedItem);
        renderShopItem(renderer, items[i], x, startY, itemWidth, itemHeight, selected);
    }
    
    // Reroll price
    int rerollPrice = calculateRerollPrice(currentWave, rerollCount);
    renderText(renderer, "Reroll Cost: ", 150, windowHeight - 150, 2);
    renderNumber(renderer, rerollPrice, 280, windowHeight - 150, 2);
    
    // Controls hint
    renderText(renderer, "Materials: ", 150, windowHeight - 120, 2);
}

void Shop::renderShopItem(SDL_Renderer* renderer, const ShopItem& item, int x, int y, int width, int height, bool selected) {
    // Item background
    if (item.locked) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 0, 255); // Yellow for locked
    } else if (selected) {
        SDL_SetRenderDrawColor(renderer, 80, 80, 120, 255); // Highlighted
    } else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255); // Normal
    }
    
    SDL_Rect itemRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &itemRect);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &itemRect);
    
    // Item name
    renderText(renderer, item.name.c_str(), x + 10, y + 10, 2);
    
    // Item description
    renderText(renderer, item.description.c_str(), x + 10, y + 40, 1);
    
    // Price
    renderText(renderer, "Price: ", x + 10, y + height - 40, 2);
    renderNumber(renderer, item.price, x + 80, y + height - 40, 2);
    
    // Lock status
    if (item.locked) {
        renderText(renderer, "LOCKED", x + 10, y + height - 20, 1);
    }
}

void Shop::handleInput(const Uint8* keyState, Player& player) {
    if (!active) return;
    
    // Buy items (1-4 keys)
    for (int i = 0; i < 4; i++) {
        SDL_Scancode key = (SDL_Scancode)(SDL_SCANCODE_1 + i);
        if (keyState[key] && !keyPressed[i]) {
            keyPressed[i] = true;
            if (i < items.size()) {
                buyItem(i, player);
            }
        } else if (!keyState[key]) {
            keyPressed[i] = false;
        }
    }
    
    // Lock item (L key)
    static bool lockKeyPressed = false;
    if (keyState[SDL_SCANCODE_L] && !lockKeyPressed) {
        lockKeyPressed = true;
        if (selectedItem < items.size()) {
            lockItem(selectedItem);
        }
    } else if (!keyState[SDL_SCANCODE_L]) {
        lockKeyPressed = false;
    }
    
    // Reroll (R key)
    static bool rerollKeyPressed = false;
    if (keyState[SDL_SCANCODE_R] && !rerollKeyPressed) {
        rerollKeyPressed = true;
        reroll(player, currentWave);
    } else if (!keyState[SDL_SCANCODE_R]) {
        rerollKeyPressed = false;
    }
    
    // Close shop (ESC key)
    static bool escKeyPressed = false;
    if (keyState[SDL_SCANCODE_ESCAPE] && !escKeyPressed) {
        escKeyPressed = true;
        closeShop();
    } else if (!keyState[SDL_SCANCODE_ESCAPE]) {
        escKeyPressed = false;
    }
}

void Shop::buyItem(int index, Player& player) {
    if (index >= items.size()) return;
    
    ShopItem& item = items[index];
    
    // Check if player has enough materials
    if (player.getStats().materials >= item.price) {
        // Check if player can hold more weapons
        if (item.type == ShopItemType::WEAPON) {
            // Create and add weapon to player
            auto weapon = std::make_unique<Weapon>(item.weaponType, item.tier);
            player.addWeapon(std::move(weapon));
            
            // Deduct materials
            player.getStats().materials -= item.price;
            
            std::cout << "Bought " << item.name << " for " << item.price << " materials" << std::endl;
            
            // Remove item from shop
            items.erase(items.begin() + index);
        }
    } else {
        std::cout << "Not enough materials! Need " << item.price << ", have " << player.getStats().materials << std::endl;
    }
}

void Shop::lockItem(int index) {
    if (index >= items.size()) return;
    
    items[index].locked = !items[index].locked;
    std::cout << "Item " << (items[index].locked ? "locked" : "unlocked") << std::endl;
}

void Shop::reroll(Player& player, int waveNumber) {
    int rerollPrice = calculateRerollPrice(waveNumber, rerollCount);
    
    if (player.getStats().materials >= rerollPrice) {
        player.getStats().materials -= rerollPrice;
        rerollCount++;
        
        // Keep locked items, regenerate others
        std::vector<ShopItem> lockedItems;
        for (const auto& item : items) {
            if (item.locked) {
                lockedItems.push_back(item);
            }
        }
        
        generateItems(waveNumber);
        
        // Add back locked items
        for (int i = 0; i < lockedItems.size() && i < items.size(); i++) {
            items[i] = lockedItems[i];
        }
        
        std::cout << "Rerolled shop for " << rerollPrice << " materials" << std::endl;
    } else {
        std::cout << "Not enough materials for reroll! Need " << rerollPrice << std::endl;
    }
}

int Shop::calculateItemPrice(WeaponType weaponType, WeaponTier tier, int waveNumber) {
    int basePrice = 20; // Base weapon price
    
    // Tier multiplier
    int tierMultiplier = (int)tier;
    basePrice *= tierMultiplier;
    
    // Wave scaling (increases with wave)
    basePrice += waveNumber * 5;
    
    return basePrice;
}

int Shop::calculateRerollPrice(int waveNumber, int rerollCount) {
    // Brotato reroll formula
    int rerollIncrease = std::max(1, (int)std::floor(0.40f * waveNumber));
    int firstRerollPrice = (int)std::floor(waveNumber * 0.75f) + rerollIncrease;
    
    return firstRerollPrice + (rerollCount * rerollIncrease);
}

void Shop::renderText(SDL_Renderer* renderer, const char* text, int x, int y, int scale) {
    if (gameRef) {
        gameRef->renderText(text, x, y, scale);
    }
}

void Shop::renderNumber(SDL_Renderer* renderer, int number, int x, int y, int scale) {
    if (gameRef) {
        gameRef->renderNumber(number, x, y, scale);
    }
}