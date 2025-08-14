#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <memory>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "ExperienceOrb.h"
#include "Material.h"
#include "Weapon.h"
#include "Shop.h"
#include "Bomb.h"

// Forward declarations
class SlimeEnemy;
class PebblinEnemy;

enum class EnemySpawnType {
    BASE,
    SLIME,
    PEBBLIN
};

struct SpawnIndicator {
    Vector2 position;
    float elapsed;
    float duration; // seconds
    EnemySpawnType enemyType;
    
    SpawnIndicator(const Vector2& pos, float dur, EnemySpawnType type)
        : position(pos), elapsed(0.0f), duration(dur), enemyType(type) {}
    
    bool isComplete() const { return elapsed >= duration; }
};

class Game {
public:
    Game();
    ~Game();
    
    bool init();
    void run();
    void cleanup();
    
    void renderNumber(int number, int x, int y, int scale = 1);
    void renderText(const char* text, int x, int y, int scale = 1);
    void renderTTFText(const char* text, int x, int y, SDL_Color color, int fontSize = 16);
    int getPlayerMaterials() const { return player ? player->getStats().materials : 0; }
    const Player* getPlayer() const { return player.get(); }
    SDL_Renderer* getRenderer() const { return renderer; }
    
    // Bomb management (public for item usage)
    void addBomb(Vector2 position, float timer, float radius, int damage);
    
private:
    void handleEvents();
    void update(float deltaTime);
    void render();
    void spawnEnemies();
    void updateSpawnIndicators(float deltaTime);
    void renderSpawnIndicators();
    void checkCollisions();
    void checkMeleeAttacks();
    void updateExperienceCollection();
    
    // Bomb management (private internal methods)
    void updateBombs(float deltaTime);
    void renderBombs();
    void checkBombExplosions();
    
    // Item input handling
    void handleItemInput(const Uint8* keyState);
    void updateMaterialCollection();
    float getMaterialDropChance() const;
    void renderUI();
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<SpawnIndicator> spawnIndicators;
    std::vector<std::unique_ptr<ExperienceOrb>> experienceOrbs;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Bomb>> bombs;
    
    float timeSinceLastSpawn;
    
    // Item input state
    bool fKeyPressed;
    bool rKeyPressed;
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
    
    // TTF Font system
    TTF_Font* defaultFont;
    
    // Telegraph duration for spawn indicators (seconds)
    float spawnTelegraphSeconds = 2.0f;
    
    static const int WINDOW_WIDTH = 1920;
    static const int WINDOW_HEIGHT = 1080;
};