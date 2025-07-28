#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

class Game {
public:
    Game();
    ~Game();
    
    bool init();
    void run();
    void cleanup();
    
private:
    void handleEvents();
    void update(float deltaTime);
    void render();
    void spawnEnemies();
    void checkCollisions();
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Bullet>> bullets;
    
    float timeSinceLastSpawn;
    int score;
    int wave;
    Vector2 mousePos; // Add mouse position tracking
    
    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;
};