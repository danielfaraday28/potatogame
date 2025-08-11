#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "ExperienceOrb.h"
#include "Material.h"
#include "Weapon.h"
#include "Shop.h"

class Game {
public:
    Game();
    ~Game();
    
    bool init();
    void run();
    void cleanup();
    
    void renderNumber(int number, int x, int y, int scale = 1);
    void renderText(const char* text, int x, int y, int scale = 1);
    
private:
    void handleEvents();
    void update(float deltaTime);
    void render();
    void spawnEnemies();
    void checkCollisions();
    void updateExperienceCollection();
    void updateMaterialCollection();
    float getMaterialDropChance() const;
    void renderUI();
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<std::unique_ptr<ExperienceOrb>> experienceOrbs;
    std::vector<std::unique_ptr<Material>> materials;
    
    float timeSinceLastSpawn;
    int score;
    int wave;
    Vector2 mousePos;
    
    // Brotato-like wave system
    float waveTimer;
    float waveDuration;
    bool waveActive;
    
    // Materials system (Brotato-style)
    int materialBag; // Materials stored for next wave
    static const int MAX_MATERIALS_ON_MAP = 50;
    
    // Shop system
    std::unique_ptr<Shop> shop;
    
    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;
};