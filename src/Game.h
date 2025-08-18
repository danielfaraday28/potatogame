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
#include "Menu.h"

// Forward declarations
class SlimeEnemy;
class PebblinEnemy;
class BossEnemy;

enum class EnemySpawnType {
    BASE,
    SLIME,
    PEBBLIN
};

enum class GameState {
    WAVE_ACTIVE,        // Normal wave gameplay
    WAVE_COMPLETED,     // Wave just ended, showing completion message
    SHOP_ACTIVE,        // Shop is open for player interaction
    SHOP_CLOSING,       // Brief pause after shop closes
    WAVE_STARTING       // Brief pause before next wave begins
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
    void addBullet(const Vector2& pos, const Vector2& vel, int dmg, bool isEnemy = false) {
        bullets.push_back(std::make_unique<Bullet>(pos, vel, dmg, isEnemy));
    }
    void addSpawnIndicator(const Vector2& pos, float duration, EnemySpawnType type) {
        spawnIndicators.emplace_back(pos, duration, type);
    }
    
    // Menu management
    void showPauseMenu();
    void showGameOverMenu();
    void restartGame();
    void exitGame();
    bool isGamePaused() const;
    
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
    
    // Wave transition state machine
    void enterState(GameState newState);
    void updateState(float deltaTime);
    void renderStateUI();
    
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
    bool shopJustClosed; // Flag to prevent ESC from leaking from shop to game
    
    // Menu system
    std::unique_ptr<Menu> menu;
    bool gameOverShown; // Flag to prevent repeated game over menu calls
    
    // TTF Font system
    TTF_Font* defaultFont;
    
    // Telegraph duration for spawn indicators (seconds)
    float spawnTelegraphSeconds = 2.0f;
    
    static const int WINDOW_WIDTH = 1920;
    static const int WINDOW_HEIGHT = 1080;
    static const int BOSS_WAVE_INTERVAL = 3;

    // Boss wave system
    bool isBossWave = false;
    std::unique_ptr<BossEnemy> boss;
    void startBossWave(int waveIndex);
    void endBossWave(bool bossDefeated);
    bool isBossWaveIndex(int wave) const { return wave > 0 && (wave % BOSS_WAVE_INTERVAL) == 0; }
    
    // Wave transition state machine
    GameState currentState;
    float stateTimer;
    float stateDuration;
    
    // Configuration constants for state durations
    static const float WAVE_COMPLETED_DURATION;
    static const float SHOP_CLOSING_DURATION;
    static const float WAVE_STARTING_DURATION;
};