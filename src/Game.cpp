#include "Game.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <SDL2/SDL_image.h>

Game::Game() : window(nullptr), renderer(nullptr), running(false), 
               timeSinceLastSpawn(0), score(0), wave(1), mousePos(0, 0) {
}

Game::~Game() {
    cleanup();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    
    window = SDL_CreateWindow("Brotato MVP", 
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    player = std::make_unique<Player>(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    running = true;
    
    return true;
}

void Game::run() {
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        handleEvents();
        update(deltaTime);
        render();
        
        SDL_Delay(16);
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            running = false;
        }
    }
    
    // Get mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    mousePos = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));
    
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    player->handleInput(keyState);
    
    // Update player's shoot direction to point towards mouse
    player->updateShootDirection(mousePos);
    
    if (keyState[SDL_SCANCODE_SPACE] && player->canShoot()) {
        bullets.push_back(std::make_unique<Bullet>(
            player->getPosition(), 
            player->getShootDirection()
        ));
        player->shoot();
    }
}

void Game::update(float deltaTime) {
    if (player->getHealth() <= 0) {
        running = false;
        return;
    }
    
    player->update(deltaTime);
    
    for (auto& bullet : bullets) {
        bullet->update(deltaTime);
    }
    
    for (auto& enemy : enemies) {
        enemy->update(deltaTime, player->getPosition());
    }
    
    spawnEnemies();
    checkCollisions();
    
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const std::unique_ptr<Bullet>& bullet) {
            return !bullet->isAlive();
        }), bullets.end());
    
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [&](const std::unique_ptr<Enemy>& enemy) {
            if (!enemy->isAlive()) {
                score += 10;
                return true;
            }
            return false;
        }), enemies.end());
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_RenderClear(renderer);
    
    player->render(renderer);
    
    for (auto& bullet : bullets) {
        bullet->render(renderer);
    }
    
    for (auto& enemy : enemies) {
        enemy->render(renderer);
    }
    
    SDL_RenderPresent(renderer);
}

void Game::spawnEnemies() {
    timeSinceLastSpawn += 0.016f;
    
    float spawnRate = 1.0f - (wave * 0.1f);
    if (spawnRate < 0.2f) spawnRate = 0.2f;
    
    if (timeSinceLastSpawn >= spawnRate) {
        timeSinceLastSpawn = 0;
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> side(0, 3);
        std::uniform_real_distribution<> pos(0.0, 1.0);
        
        Vector2 spawnPos;
        int spawnSide = side(gen);
        
        switch (spawnSide) {
            case 0:
                spawnPos = Vector2(-20, pos(gen) * WINDOW_HEIGHT);
                break;
            case 1:
                spawnPos = Vector2(WINDOW_WIDTH + 20, pos(gen) * WINDOW_HEIGHT);
                break;
            case 2:
                spawnPos = Vector2(pos(gen) * WINDOW_WIDTH, -20);
                break;
            case 3:
                spawnPos = Vector2(pos(gen) * WINDOW_WIDTH, WINDOW_HEIGHT + 20);
                break;
        }
        
        enemies.push_back(std::make_unique<Enemy>(spawnPos, renderer));
    }
}

void Game::checkCollisions() {
    for (auto& bullet : bullets) {
        for (auto& enemy : enemies) {
            if (bullet->isAlive() && enemy->isAlive()) {
                float distance = bullet->getPosition().distance(enemy->getPosition());
                if (distance < bullet->getRadius() + enemy->getRadius()) {
                    bullet->destroy();
                    enemy->hit(); // Trigger hit animation
                    enemy->destroy();
                }
            }
        }
    }
    
    for (auto& enemy : enemies) {
        if (enemy->isAlive()) {
            float distance = player->getPosition().distance(enemy->getPosition());
            if (distance < player->getRadius() + enemy->getRadius()) {
                player->takeDamage(enemy->getDamage());
                enemy->destroy();
            }
        }
    }
}

void Game::cleanup() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    IMG_Quit();
    SDL_Quit();
}