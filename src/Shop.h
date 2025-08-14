#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <string>
#include "Weapon.h"
#include "Item.h"

class Player;
class Game;

enum class ShopItemType {
    WEAPON,
    ITEM  // For future items
};

struct ShopItem {
    ShopItemType type;
    WeaponType weaponType;
    WeaponTier tier;
    ItemType itemType;
    int itemPower;
    int price;
    bool locked;
    std::string name;
    std::string description;
    
    ShopItem(WeaponType wType, WeaponTier wTier, int cost) 
        : type(ShopItemType::WEAPON), weaponType(wType), tier(wTier), 
          price(cost), locked(false), itemType(ItemType::HEALING_BOX), itemPower(0) {
        
        // Generate name and description
        name = getWeaponName(wType, wTier);
        description = getWeaponDescription(wType, wTier);
    }
    
    ShopItem(ItemType iType, int power, int cost)
        : type(ShopItemType::ITEM), weaponType(WeaponType::PISTOL), tier(WeaponTier::TIER_1),
          price(cost), locked(false), itemType(iType), itemPower(power) {
        
        // Generate name and description
        name = getItemName(iType);
        description = getItemDescription(iType, power);
    }
    
private:
    std::string getWeaponName(WeaponType wType, WeaponTier wTier);
    std::string getWeaponDescription(WeaponType wType, WeaponTier wTier);
    std::string getItemName(ItemType iType);
    std::string getItemDescription(ItemType iType, int power);
};

class Shop {
public:
    Shop();
    ~Shop();
    
    void setGame(Game* game) { gameRef = game; }
    void loadAssets(SDL_Renderer* renderer);
    void unloadAssets();
    
    void generateItems(int waveNumber, int playerLuck = 0);
    void render(SDL_Renderer* renderer, int windowWidth, int windowHeight);
    void addItemToShop(ItemType type, int power, int waveNumber);
    int calculateItemPrice(ItemType type, int power, int waveNumber);
    void handleInput(const Uint8* keyState, Player& player);
    void handleMouseInput(int mouseX, int mouseY, bool mousePressed, Player& player);
    
    bool isShopActive() const { return active; }
    void openShop(int waveNumber);
    void closeShop();
    
    // Shop actions
    void buyItem(int index, Player& player);
    void sellOwnedWeapon(int ownedIndex, Player& player);
    void lockItem(int index);
    void reroll(Player& player, int waveNumber);
    
private:
    bool active;
    std::vector<ShopItem> items;
    int rerollCount;
    int currentWave;
    
    // UI helpers
    void renderShopItem(SDL_Renderer* renderer, const ShopItem& item, int x, int y, int width, int height, bool highlighted, int index);
    void renderCharacterStats(SDL_Renderer* renderer, int x, int y, int width, int height);
    void renderText(SDL_Renderer* renderer, const char* text, int x, int y, int scale = 1);
    void renderNumber(SDL_Renderer* renderer, int number, int x, int y, int scale = 1);
    void renderTTFText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, int fontSize = 16);
    SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);
    
    // Shop logic
    int calculateItemPrice(WeaponType weaponType, WeaponTier tier, int waveNumber);
    int calculateRerollPrice(int waveNumber, int rerollCount);
    bool canItemAppear(WeaponType weaponType, WeaponTier tier, int waveNumber);
    
    // Input handling
    int selectedItem;
    bool keyPressed[4]; // Track key states to prevent multiple presses
    int hoveredItem;
    bool lastMousePressed;
    int selectedOwnedWeapon; // index of selected owned weapon in player's array (-1 if none)
    int hoveredOwnedWeapon;  // hover index for owned weapons
    
    Game* gameRef = nullptr; // Reference to game for text rendering
    
    static const int MAX_SHOP_ITEMS = 4;

    // UI textures
    SDL_Texture* texCardNormal = nullptr;
    SDL_Texture* texCardSelected = nullptr;
    SDL_Texture* texCardLocked = nullptr;
    SDL_Texture* texCoin = nullptr;
    SDL_Texture* texLock = nullptr;
    SDL_Texture* texLockLocked = nullptr;
    SDL_Texture* texReroll = nullptr;
    SDL_Texture* texWeaponPistol = nullptr;
    SDL_Texture* texWeaponSMG = nullptr;
    SDL_Texture* texHealingBox = nullptr;
    SDL_Texture* texMassBomb = nullptr;
};