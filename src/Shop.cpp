#include "Shop.h"
#include "Player.h"
#include "Game.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
#include <cstring>
#include <sstream>
#include <SDL2/SDL_image.h>

std::string ShopItem::getWeaponName(WeaponType wType, WeaponTier wTier) {
    std::string baseName;
    switch (wType) {
        case WeaponType::PISTOL: baseName = "Pistol"; break;
        case WeaponType::SMG: baseName = "SMG"; break;
        case WeaponType::MELEE_STICK: baseName = "Brick on Stick"; break;
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
        case WeaponType::MELEE_STICK:
            return "Melee weapon with short range";
    }
    return "";
}

std::string ShopItem::getItemName(ItemType iType) {
    switch (iType) {
        case ItemType::HEALING_BOX:
            return "HEALING BOX";
        case ItemType::MASS_BOMB:
            return "MASS BOMB";
    }
    return "";
}

std::string ShopItem::getItemDescription(ItemType iType, int power) {
    std::stringstream ss;
    switch (iType) {
        case ItemType::HEALING_BOX:
            ss << "Restores " << power << " health";
            break;
        case ItemType::MASS_BOMB:
            ss << "Deals " << power << " damage\nRadius: " << (power * 5);
            break;
    }
    return ss.str();
}

Shop::Shop() : active(false), rerollCount(0), currentWave(0), selectedItem(0), hoveredItem(-1), lastMousePressed(false), selectedOwnedWeapon(-1), hoveredOwnedWeapon(-1) {
    for (int i = 0; i < 4; i++) {
        keyPressed[i] = false;
    }
}

Shop::~Shop() {
    unloadAssets();
}

void Shop::loadAssets(SDL_Renderer* renderer) {
    // Load UI textures. These are optional; if missing we fall back to rects.
    texCardNormal = loadTexture("assets/ui/card_normal.png", renderer);
    texCardSelected = loadTexture("assets/ui/card_selected.png", renderer);
    texCardLocked = loadTexture("assets/ui/card_locked.png", renderer);
    texCoin = loadTexture("assets/ui/coin.png", renderer);
    texLock = loadTexture("assets/ui/lock.png", renderer);
    texLockLocked = loadTexture("assets/ui/lock_locked.png", renderer);
    texReroll = loadTexture("assets/ui/reroll.png", renderer);
    texWeaponPistol = loadTexture("assets/weapons/pistol.png", renderer);
    texWeaponSMG = loadTexture("assets/weapons/smg.png", renderer);
    texHealingBox = loadTexture("assets/items/healing_box.png", renderer);
    texMassBomb = loadTexture("assets/items/mass_bomb.png", renderer);
}

void Shop::unloadAssets() {
    auto destroyIf = [](SDL_Texture*& t) {
        if (t) { SDL_DestroyTexture(t); t = nullptr; }
    };
    destroyIf(texCardNormal);
    destroyIf(texCardSelected);
    destroyIf(texCardLocked);
    destroyIf(texCoin);
    destroyIf(texLock);
    destroyIf(texLockLocked);
    destroyIf(texReroll);
    destroyIf(texWeaponPistol);
    destroyIf(texWeaponSMG);
    destroyIf(texHealingBox);
    destroyIf(texMassBomb);
}

void Shop::generateItems(int waveNumber, int playerLuck) {
    items.clear();
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    // Generate 4 random items
    for (int i = 0; i < MAX_SHOP_ITEMS; i++) {
        // 70% chance for weapon, 30% for item
        std::uniform_real_distribution<float> typeDist(0.0f, 1.0f);
        if (typeDist(gen) < 0.7f) {
            // Generate weapon
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
        } else {
            // Generate item
            std::uniform_int_distribution<int> itemDist(0, 1);
            ItemType itemType = (itemDist(gen) == 0) ? ItemType::HEALING_BOX : ItemType::MASS_BOMB;
            
            // Scale item power with wave
            int basePower = 20;
            int power = basePower + (waveNumber * 5);
            
            int price = calculateItemPrice(itemType, power, waveNumber);
            items.emplace_back(itemType, power, price);
        }
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
    
    // Proper shop layout - centered with room for stats panel on right
    int statsWidth = 350; // Fixed width for stats panel
    int margin = 50; // Margins around elements
    int spacing = 20; // Space between shop and stats
    
    int availableWidth = windowWidth - (2 * margin) - statsWidth - spacing;
    int shopWidth = availableWidth; // Use remaining space for shop
    int shopHeight = 700;
    
    int shopX = margin; // Start with proper margin
    int shopY = (windowHeight - shopHeight) / 2; // Center vertically
    
    // Draw shop background panel
    SDL_SetRenderDrawColor(renderer, 20, 22, 30, 240);
    SDL_Rect shopBg = {shopX, shopY, shopWidth, shopHeight};
    SDL_RenderFillRect(renderer, &shopBg);
    SDL_SetRenderDrawColor(renderer, 100, 120, 150, 255);
    SDL_RenderDrawRect(renderer, &shopBg);
    
    // Title section - use TTF font
    SDL_Color white = {255, 255, 255, 255};
    renderTTFText(renderer, "WEAPON SHOP", shopX + 20, shopY + 20, white, 24);
    
    // Current materials display
    renderTTFText(renderer, "MATERIALS:", shopX + shopWidth - 200, shopY + 20, white, 16);
    if (gameRef) {
        std::string materialsText = std::to_string(gameRef->getPlayerMaterials());
        renderTTFText(renderer, materialsText.c_str(), shopX + shopWidth - 70, shopY + 20, white, 20);
    }
    
    // Instructions
    renderTTFText(renderer, "CLICK TO BUY ITEMS  |  R=REROLL  |  ESC=CLOSE", shopX + 20, shopY + 55, white, 16);
    
    // Items in a horizontal row - properly sized to fit 4 items
    int itemSpacing = 20;
    int itemWidth = (shopWidth - 40 - (3 * itemSpacing)) / 4; // Fit 4 items with spacing
    int itemHeight = 300;
    int gridStartX = shopX + 20; // Start with proper margin inside shop
    int gridStartY = shopY + 120;
    
    for (int i = 0; i < items.size() && i < 4; i++) {
        // Horizontal layout - all items in one row
        int x = gridStartX + i * (itemWidth + itemSpacing);
        int y = gridStartY;
        
        bool hovered = (i == hoveredItem);
        bool selected = (i == selectedItem);
        renderShopItem(renderer, items[i], x, y, itemWidth, itemHeight, selected || hovered, i);
    }

    // Bottom-left Items inventory section (display slots like Brotato)
    int itemsPanelY = shopY + shopHeight - 160;
    renderTTFText(renderer, "Items", shopX + 20, itemsPanelY - 28, white, 18);

    // Draw 6 slots (placeholders until items are implemented)
    int slotSize = 56;
    int slotSpacing = 14;
    int slotsToShow = 6;
    for (int i = 0; i < slotsToShow; i++) {
        int sx = shopX + 20 + i * (slotSize + slotSpacing);
        int sy = itemsPanelY;
        SDL_Rect slotRect = {sx, sy, slotSize, slotSize};
        SDL_SetRenderDrawColor(renderer, 45, 50, 60, 255);
        SDL_RenderFillRect(renderer, &slotRect);
        SDL_SetRenderDrawColor(renderer, 120, 130, 150, 255);
        SDL_RenderDrawRect(renderer, &slotRect);
    }

    // Bottom-right: Owned weapons grid (player weapon array)
    int weaponsPanelX = shopX + shopWidth - 320; // inside shop
    int weaponsPanelY = itemsPanelY;
    renderTTFText(renderer, "Weapons (click to sell for 50%)", weaponsPanelX, weaponsPanelY - 28, white, 18);
    int wSlotSize = 56;
    int wSlotSpacing = 14;
    if (gameRef) {
        const Player* p = gameRef->getPlayer();
        if (p) {
            const auto& owned = p->getWeapons();
            for (int i = 0; i < 6; i++) {
                int cx = weaponsPanelX + i * (wSlotSize + wSlotSpacing);
                int cy = weaponsPanelY;
                SDL_Rect slot = {cx, cy, wSlotSize, wSlotSize};
                SDL_SetRenderDrawColor(renderer, 45, 50, 60, 255);
                SDL_RenderFillRect(renderer, &slot);
                SDL_SetRenderDrawColor(renderer, 120, 130, 150, 255);
                SDL_RenderDrawRect(renderer, &slot);
                if (i < (int)owned.size()) {
                    SDL_Texture* icon = nullptr;
                    switch (owned[i]->getType()) {
                        case WeaponType::PISTOL: icon = texWeaponPistol; break;
                        case WeaponType::SMG: icon = texWeaponSMG; break;
                        case WeaponType::MELEE_STICK: icon = nullptr; break; // No texture yet
                    }
                    if (icon) {
                        SDL_Rect ir = {cx + 8, cy + 8, wSlotSize - 16, wSlotSize - 16};
                        SDL_RenderCopy(renderer, icon, nullptr, &ir);
                    }
                    // Highlight if selected/hovered
                    if (selectedOwnedWeapon == i || hoveredOwnedWeapon == i) {
                        SDL_SetRenderDrawColor(renderer, 200, 220, 255, 255);
                        SDL_RenderDrawRect(renderer, &slot);
                    }
                }
            }

            // Show sell price tooltip when hovering an owned weapon
            if (hoveredOwnedWeapon >= 0 && hoveredOwnedWeapon < (int)owned.size()) {
                WeaponType t = owned[hoveredOwnedWeapon]->getType();
                WeaponTier tr = owned[hoveredOwnedWeapon]->getTier();
                int price = calculateItemPrice(t, tr, currentWave);
                int sellValue = std::max(1, price / 2);
                SDL_Color yellow = {255, 220, 120, 255};
                std::string tip = "Sell for: " + std::to_string(sellValue);
                renderTTFText(renderer, tip.c_str(), weaponsPanelX, weaponsPanelY + wSlotSize + 14, yellow, 14);
            }
        }
    }
    
    // Bottom section with reroll and close buttons - move them out of the way
    int bottomY = shopY + shopHeight - 100;
    
    // Reroll button - move to bottom left
    SDL_Rect rerollButton = {shopX + 50, bottomY, 150, 50};
    int rerollPrice = calculateRerollPrice(currentWave, rerollCount);
    bool canAffordReroll = gameRef && gameRef->getPlayerMaterials() >= rerollPrice;
    
    SDL_SetRenderDrawColor(renderer, canAffordReroll ? 60 : 40, canAffordReroll ? 80 : 50, canAffordReroll ? 100 : 60, 255);
    SDL_RenderFillRect(renderer, &rerollButton);
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &rerollButton);
    
    // Use TTF text for reroll button
    renderTTFText(renderer, "REROLL", rerollButton.x + 10, rerollButton.y + 8, white, 14);
    std::string rerollPriceText = std::to_string(rerollPrice);
    renderTTFText(renderer, rerollPriceText.c_str(), rerollButton.x + 10, rerollButton.y + 25, white, 14);
    
    // Close button - move to bottom right
    SDL_Rect closeButton = {shopX + shopWidth - 200, bottomY, 150, 50};
    SDL_SetRenderDrawColor(renderer, 80, 60, 60, 255);
    SDL_RenderFillRect(renderer, &closeButton);
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &closeButton);
    
    // Use TTF text for close button
    renderTTFText(renderer, "CLOSE", closeButton.x + 20, closeButton.y + 12, white, 14);
    
    // Character stats panel on the right side - properly positioned and sized
    int statsX = shopX + shopWidth + spacing;
    renderCharacterStats(renderer, statsX, shopY, statsWidth, shopHeight);
}

void Shop::renderShopItem(SDL_Renderer* renderer, const ShopItem& item, int x, int y, int width, int height, bool highlighted, int index) {
    SDL_Rect itemRect = {x, y, width, height};
    
    // Check if player can afford this item
    bool canAfford = gameRef && gameRef->getPlayerMaterials() >= item.price;

    // Card background with better colors
    if (item.locked) {
        SDL_SetRenderDrawColor(renderer, 120, 100, 50, 255);
    } else if (highlighted) {
        SDL_SetRenderDrawColor(renderer, canAfford ? 80 : 60, canAfford ? 120 : 80, canAfford ? 160 : 100, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, canAfford ? 60 : 40, canAfford ? 70 : 50, canAfford ? 90 : 60, 255);
    }
    SDL_RenderFillRect(renderer, &itemRect);
    
    // Border
    if (highlighted) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 120, 120, 140, 255);
    }
    SDL_RenderDrawRect(renderer, &itemRect);
    
    // Item icon - larger and centered
    SDL_Texture* icon = nullptr;
    if (item.type == ShopItemType::WEAPON) {
        switch (item.weaponType) {
            case WeaponType::PISTOL: icon = texWeaponPistol; break;
            case WeaponType::SMG: icon = texWeaponSMG; break;
            case WeaponType::MELEE_STICK: icon = nullptr; break; // No texture yet
        }
    } else if (item.type == ShopItemType::ITEM) {
        switch (item.itemType) {
            case ItemType::HEALING_BOX: icon = texHealingBox; break;
            case ItemType::MASS_BOMB: icon = texMassBomb; break;
        }
    }
    if (icon) {
        SDL_Rect ir = {x + width/2 - 24, y + 10, 48, 48};
        SDL_RenderCopy(renderer, icon, nullptr, &ir);
    }

    // Item name - use TTF text
    SDL_Color itemWhite = {255, 255, 255, 255};
    renderTTFText(renderer, item.name.c_str(), x + 10, y + 65, itemWhite, 14);
    
    // Price with BUY button
    int buttonY = y + height - 50;
    SDL_Rect buyButton = {x + 10, buttonY, width - 20, 30};
    
    if (canAfford) {
        SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 50, 50, 255);
    }
    SDL_RenderFillRect(renderer, &buyButton);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &buyButton);
    
    // BUY text and price - use TTF text
    SDL_Color buttonWhite = {255, 255, 255, 255};
    if (canAfford) {
        renderTTFText(renderer, "BUY", buyButton.x + 10, buyButton.y + 8, buttonWhite, 14);
    } else {
        renderTTFText(renderer, "NEED", buyButton.x + 5, buyButton.y + 8, buttonWhite, 14);
    }
    
    // Price
    std::string priceText = std::to_string(item.price);
    renderTTFText(renderer, priceText.c_str(), buyButton.x + buyButton.w - 40, buyButton.y + 8, buttonWhite, 14);
    
    // Small lock indicator if locked
    if (item.locked) {
        SDL_Rect lockRect = {x + width - 25, y + 5, 20, 20};
        SDL_SetRenderDrawColor(renderer, 200, 200, 50, 255);
        SDL_RenderFillRect(renderer, &lockRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &lockRect);
    }
}

void Shop::renderCharacterStats(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (!gameRef) return;
    
    // Draw stats panel background
    SDL_SetRenderDrawColor(renderer, 30, 35, 45, 240);
    SDL_Rect statsPanel = {x, y, width, height};
    SDL_RenderFillRect(renderer, &statsPanel);
    SDL_SetRenderDrawColor(renderer, 120, 140, 160, 255);
    SDL_RenderDrawRect(renderer, &statsPanel);
    
    // Stats title
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {100, 255, 100, 255};
    SDL_Color red = {255, 100, 100, 255};
    SDL_Color blue = {100, 150, 255, 255};
    
    renderTTFText(renderer, "STATS", x + 20, y + 20, white, 20);
    
    // Get player stats
    const Player* player = gameRef->getPlayer();
    if (!player) return;
    
    const PlayerStats& stats = player->getStats();
    int currentY = y + 60;
    int lineHeight = 25;
    
    // Health
    std::string healthText = "Max HP: " + std::to_string(stats.maxHealth);
    renderTTFText(renderer, healthText.c_str(), x + 20, currentY, red, 14);
    currentY += lineHeight;
    
    // Damage
    std::string damageText = "Damage: " + std::to_string(stats.damage);
    renderTTFText(renderer, damageText.c_str(), x + 20, currentY, red, 14);
    currentY += lineHeight;
    
    // Attack Speed
    std::string attackSpeedText = "Attack Speed: " + std::to_string((int)(stats.attackSpeed * 100)) + "%";
    renderTTFText(renderer, attackSpeedText.c_str(), x + 20, currentY, green, 14);
    currentY += lineHeight;
    
    // Skip speed for now since it's not in PlayerStats
    // std::string speedText = "Speed: " + std::to_string(stats.speed);
    // renderTTFText(renderer, speedText.c_str(), x + 20, currentY, blue, 14);
    // currentY += lineHeight;
    
    // Armor
    std::string armorText = "Armor: " + std::to_string(stats.armor);
    renderTTFText(renderer, armorText.c_str(), x + 20, currentY, blue, 14);
    currentY += lineHeight;
    
    // Dodge
    std::string dodgeText = "Dodge: " + std::to_string((int)stats.dodgeChance) + "%";
    renderTTFText(renderer, dodgeText.c_str(), x + 20, currentY, green, 14);
    currentY += lineHeight;
    
    // Materials
    std::string materialsText = "Materials: " + std::to_string(stats.materials);
    renderTTFText(renderer, materialsText.c_str(), x + 20, currentY, green, 14);
    currentY += lineHeight + 10;
    
    // Weapons section
    std::string weaponsTitle = "WEAPONS (" + std::to_string(player->getWeaponCount()) + "/6)";
    renderTTFText(renderer, weaponsTitle.c_str(), x + 20, currentY, white, 16);
    currentY += 30;
    
    // List equipped weapons
    const auto& weapons = player->getWeapons();
    for (int i = 0; i < weapons.size() && i < 6; i++) {
        std::string weaponText = "- ";
        switch (weapons[i]->getType()) {
            case WeaponType::PISTOL: weaponText += "Pistol"; break;
            case WeaponType::SMG: weaponText += "SMG"; break;
            case WeaponType::MELEE_STICK: weaponText += "Brick on Stick"; break;
        }
        
        // Add tier info
        switch (weapons[i]->getTier()) {
            case WeaponTier::TIER_1: weaponText += " I"; break;
            case WeaponTier::TIER_2: weaponText += " II"; break;
            case WeaponTier::TIER_3: weaponText += " III"; break;
            case WeaponTier::TIER_4: weaponText += " IV"; break;
        }
        
        renderTTFText(renderer, weaponText.c_str(), x + 30, currentY, white, 12);
        currentY += 20;
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
        bool purchased = false;
        
        if (item.type == ShopItemType::WEAPON) {
            // Create and add weapon to player with renderer for sprite loading
            auto weapon = std::make_unique<Weapon>(item.weaponType, item.tier);
            player.addWeapon(std::move(weapon), gameRef->getRenderer());
            purchased = true;
        } else if (item.type == ShopItemType::ITEM) {
            // Create and add item to player
            std::unique_ptr<Item> newItem;
            switch (item.itemType) {
                case ItemType::HEALING_BOX:
                    newItem = std::make_unique<HealingBox>(item.itemPower, item.price);
                    break;
                case ItemType::MASS_BOMB:
                    newItem = std::make_unique<MassBomb>(item.itemPower, 3.0f, item.itemPower * 5.0f, item.price);
                    break;
            }
            if (newItem) {
                player.addItem(std::move(newItem));
                purchased = true;
            }
        }
        
        if (purchased) {
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

void Shop::sellOwnedWeapon(int ownedIndex, Player& player) {
    if (!gameRef) return;
    const auto& owned = player.getWeapons();
    if (ownedIndex < 0 || ownedIndex >= static_cast<int>(owned.size())) return;

    WeaponType type = owned[ownedIndex]->getType();
    WeaponTier tier = owned[ownedIndex]->getTier();
    int price = calculateItemPrice(type, tier, currentWave);
    int sellValue = std::max(1, price / 2);

    player.getStats().materials += sellValue;
    player.removeWeaponAt(ownedIndex);
    std::cout << "Sold weapon for " << sellValue << " materials" << std::endl;
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

void Shop::addItemToShop(ItemType type, int power, int waveNumber) {
    int price = calculateItemPrice(type, power, waveNumber);
    items.emplace_back(type, power, price);
}

int Shop::calculateItemPrice(ItemType type, int power, int waveNumber) {
    int basePrice = 15; // Base item price (slightly cheaper than weapons)
    
    // Power scaling
    basePrice += power / 2;
    
    // Wave scaling
    basePrice += waveNumber * 3;
    
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

void Shop::renderTTFText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, int fontSize) {
    if (gameRef) {
        gameRef->renderTTFText(text, x, y, color, fontSize);
    }
}

SDL_Texture* Shop::loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cout << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        std::cout << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
    }
    return texture;
}

void Shop::handleMouseInput(int mouseX, int mouseY, bool mousePressed, Player& player) {
    if (!active) return;
    
    hoveredItem = -1;
    
    // Calculate shop layout (same as in render method)
    int windowWidth = 1920;
    int windowHeight = 1080;
    
    // Same layout calculation as render method
    int statsWidth = 350; // Fixed width for stats panel
    int margin = 50; // Margins around elements
    int spacing = 20; // Space between shop and stats
    
    int availableWidth = windowWidth - (2 * margin) - statsWidth - spacing;
    int shopWidth = availableWidth; // Use remaining space for shop
    int shopHeight = 700;
    
    int shopX = margin; // Start with proper margin
    int shopY = (windowHeight - shopHeight) / 2; // Center vertically
    
    // Items in horizontal row - same calculation as render
    int itemSpacing = 20;
    int itemWidth = (shopWidth - 40 - (3 * itemSpacing)) / 4; // Fit 4 items with spacing
    int itemHeight = 300;
    int gridStartX = shopX + 20; // Start with proper margin inside shop
    int gridStartY = shopY + 120;
    
    // Check item hover/click (make whole card clickable like Brotato)
    for (int i = 0; i < items.size() && i < 4; i++) {
        int x = gridStartX + i * (itemWidth + itemSpacing);
        int y = gridStartY;
        
        SDL_Rect itemRect = {x, y, itemWidth, itemHeight};
        
        if (mouseX >= itemRect.x && mouseX <= itemRect.x + itemRect.w &&
            mouseY >= itemRect.y && mouseY <= itemRect.y + itemRect.h) {
            hoveredItem = i;
            
            // Handle click to buy
            if (mousePressed && !lastMousePressed) {
                buyItem(i, player);
            }
        }
    }
    
    // Check reroll button
    int bottomY = shopY + shopHeight - 100;
    SDL_Rect rerollButton = {shopX + 50, bottomY, 150, 50};
    if (mouseX >= rerollButton.x && mouseX <= rerollButton.x + rerollButton.w &&
        mouseY >= rerollButton.y && mouseY <= rerollButton.y + rerollButton.h) {
        if (mousePressed && !lastMousePressed) {
            int rerollPrice = calculateRerollPrice(currentWave, rerollCount);
            if (gameRef && gameRef->getPlayerMaterials() >= rerollPrice) {
                reroll(player, currentWave);
            }
        }
    }
    
    // Check close button
    SDL_Rect closeButton = {shopX + shopWidth - 200, bottomY, 150, 50};
    if (mouseX >= closeButton.x && mouseX <= closeButton.x + closeButton.w &&
        mouseY >= closeButton.y && mouseY <= closeButton.y + closeButton.h) {
        if (mousePressed && !lastMousePressed) {
            closeShop();
        }
    }

    // Click detection for owned weapons (bottom-right panel)
    // Use existing layout variables from above
    int itemsPanelY2 = shopY + shopHeight - 160;
    int weaponsPanelX2 = shopX + shopWidth - 320;
    int wSlotSize = 56; int wSlotSpacing = 14;
    selectedOwnedWeapon = -1;
    hoveredOwnedWeapon = -1;
    const auto& owned = player.getWeapons();
    for (int i = 0; i < 6; i++) {
        int cx = weaponsPanelX2 + i * (wSlotSize + wSlotSpacing);
        int cy = itemsPanelY2;
        SDL_Rect slot = {cx, cy, wSlotSize, wSlotSize};
        if (mouseX >= slot.x && mouseX <= slot.x + slot.w && mouseY >= slot.y && mouseY <= slot.y + slot.h) {
            hoveredOwnedWeapon = i;
            if (mousePressed && !lastMousePressed) {
                if (i < (int)owned.size()) {
                    // Click to sell at 50%
                    sellOwnedWeapon(i, player);
                }
            }
        }
    }
    
    lastMousePressed = mousePressed;
}