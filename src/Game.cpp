#include "Game.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <SDL2/SDL_image.h>
#include <sstream>
#include <iomanip>
#include <cstdio> // Add for sprintf
#include <string>
#include <map>

Game::Game() : window(nullptr), renderer(nullptr), running(false), 
               timeSinceLastSpawn(0), score(0), wave(1), mousePos(0, 0),
               waveTimer(0), waveDuration(20.0f), waveActive(true), materialBag(0) {
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
    shop = std::make_unique<Shop>();
    shop->setGame(this);
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
    
    // Handle shop input if shop is active
    if (shop->isShopActive()) {
        shop->handleInput(keyState, *player);
    } else {
        player->handleInput(keyState);
        
        // Update player's shoot direction to point towards mouse
        player->updateShootDirection(mousePos);
    }
}

void Game::update(float deltaTime) {
    if (player->getHealth() <= 0) {
        running = false;
        return;
    }
    
    // Don't update game if shop is active
    if (shop->isShopActive()) {
        return;
    }
    
    // Update wave timer
    if (waveActive) {
        waveTimer += deltaTime;
        if (waveTimer >= waveDuration) {
            // Wave completed - distribute bagged materials
            if (materialBag > 0) {
                player->gainMaterials(materialBag);
                std::cout << "Collected " << materialBag << " materials from bag!" << std::endl;
                materialBag = 0;
            }
            
            // Open shop after wave completion
            shop->openShop(wave);
            
            // Prepare for next wave
            wave++;
            waveTimer = 0;
            std::cout << "Wave " << wave << " will start after shop" << std::endl;
            
            // Increase wave duration by 5 seconds each wave, capped at 60 seconds
            if (waveDuration < 60.0f) {
                waveDuration += 5.0f;
                if (waveDuration > 60.0f) waveDuration = 60.0f;
            }
        }
    }
    
    player->update(deltaTime);
    
    // Update weapons (they will fire in aim direction)
    player->updateWeapons(deltaTime, bullets);
    
    for (auto& bullet : bullets) {
        bullet->update(deltaTime);
    }
    
    for (auto& enemy : enemies) {
        enemy->update(deltaTime, player->getPosition());
    }
    
    for (auto& orb : experienceOrbs) {
        orb->update(deltaTime);
    }
    
    for (auto& material : materials) {
        material->update(deltaTime);
    }
    
    spawnEnemies();
    checkCollisions();
    updateExperienceCollection();
    updateMaterialCollection();
    
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const std::unique_ptr<Bullet>& bullet) {
            return !bullet->isAlive();
        }), bullets.end());
    
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [&](const std::unique_ptr<Enemy>& enemy) {
            if (!enemy->isAlive()) {
                // Brotato-style material drop system
                float dropChance = getMaterialDropChance();
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                
                if (dist(gen) < dropChance) {
                    // Check material limit on map
                    if (materials.size() < MAX_MATERIALS_ON_MAP) {
                        int matValue = 1 + (wave / 3);
                        int expValue = 1 + (wave / 5);
                        materials.push_back(std::make_unique<Material>(
                            enemy->getPosition(), matValue, expValue
                        ));
                    } else {
                        // Add to bag if map is full
                        materialBag += 1 + (wave / 3);
                    }
                }
                
                score += 10;
                return true;
            }
            return false;
        }), enemies.end());
    
    experienceOrbs.erase(std::remove_if(experienceOrbs.begin(), experienceOrbs.end(),
        [](const std::unique_ptr<ExperienceOrb>& orb) {
            return !orb->isAlive();
        }), experienceOrbs.end());
    
    materials.erase(std::remove_if(materials.begin(), materials.end(),
        [&](const std::unique_ptr<Material>& material) {
            if (!material->isAlive()) {
                // Add uncollected materials to bag
                materialBag += material->getMaterialValue();
                return true;
            }
            return false;
        }), materials.end());
}

void Game::updateExperienceCollection() {
    Vector2 playerPos = player->getPosition();
    float pickupRange = player->getStats().pickupRange;
    
    for (auto& orb : experienceOrbs) {
        if (orb->isAlive()) {
            float distance = playerPos.distance(orb->getPosition());
            if (distance <= pickupRange) {
                player->gainExperience(orb->getExperienceValue());
                orb->collect();
            }
        }
    }
}

void Game::updateMaterialCollection() {
    Vector2 playerPos = player->getPosition();
    float pickupRange = player->getStats().pickupRange;
    
    for (auto& material : materials) {
        if (material->isAlive()) {
            float distance = playerPos.distance(material->getPosition());
            if (distance <= pickupRange) {
                // Materials provide both experience and gold/materials
                player->gainExperience(material->getExperienceValue());
                player->gainMaterials(material->getMaterialValue());
                material->collect();
            }
        }
    }
}

float Game::getMaterialDropChance() const {
    // Brotato's material drop formula:
    // Starts at 100%, decreases 1.5% per wave number, minimum 50%
    float baseChance = 1.0f;
    float reduction = (wave - 1) * 0.015f;
    float dropChance = baseChance - reduction;
    
    // Minimum 50% chance
    if (dropChance < 0.5f) dropChance = 0.5f;
    
    // TODO: Implement horde wave reduction (-35%)
    
    return dropChance;
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_RenderClear(renderer);
    
    player->render(renderer);
    player->renderWeapons(renderer);
    
    for (auto& bullet : bullets) {
        bullet->render(renderer);
    }
    
    for (auto& enemy : enemies) {
        enemy->render(renderer);
    }
    
    for (auto& orb : experienceOrbs) {
        orb->render(renderer);
    }
    
    for (auto& material : materials) {
        material->render(renderer);
    }
    
    renderUI();
    
    // Render shop on top if active
    shop->render(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    SDL_RenderPresent(renderer);
}

void Game::renderUI() {
    // === BROTATO-STYLE UI LAYOUT WITH BITMAP TEXT ===
    
    // Top-left: Health bar with actual numbers
    SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255); // Dark red background
    SDL_Rect healthBg = {20, 20, 200, 35};
    SDL_RenderFillRect(renderer, &healthBg);
    
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red health bar
    int healthWidth = (player->getHealth() * 200) / player->getStats().maxHealth;
    SDL_Rect healthBar = {20, 20, healthWidth, 35};
    SDL_RenderFillRect(renderer, &healthBar);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
    SDL_RenderDrawRect(renderer, &healthBg);
    
    // Health text "X / Y"
    renderNumber(player->getHealth(), 30, 28, 2);
    renderText(" / ", 30 + std::to_string(player->getHealth()).length() * 12, 28, 2);
    renderNumber(player->getStats().maxHealth, 30 + std::to_string(player->getHealth()).length() * 12 + 24, 28, 2);
    
    // Top-left: Level display "LV.X"
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255); // Dark gray background
    SDL_Rect levelBg = {20, 65, 120, 30};
    SDL_RenderFillRect(renderer, &levelBg);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
    SDL_RenderDrawRect(renderer, &levelBg);
    
    // Level text
    renderText("LV.", 30, 72, 2);
    renderNumber(player->getLevel(), 54, 72, 2);
    
    // Top-left: Materials counter (green circle with actual number)
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // Dark green
    // Draw circle
    for (int x = -40; x <= 40; x++) {
        for (int y = -40; y <= 40; y++) {
            if (x*x + y*y <= 40*40) {
                SDL_RenderDrawPoint(renderer, 70 + x, 150 + y);
            }
        }
    }
    
    SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255); // Brighter green inner circle
    for (int x = -35; x <= 35; x++) {
        for (int y = -35; y <= 35; y++) {
            if (x*x + y*y <= 35*35) {
                SDL_RenderDrawPoint(renderer, 70 + x, 150 + y);
            }
        }
    }
    
    // Materials number (centered in circle)
    int materialDigits = std::to_string(player->getStats().materials).length();
    int materialX = 70 - (materialDigits * 6); // Center the number
    renderNumber(player->getStats().materials, materialX, 142, 2);
    
    // Center top: Wave number with actual text
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Semi-transparent black
    SDL_Rect waveBg = {WINDOW_WIDTH/2 - 80, 20, 160, 40};
    SDL_RenderFillRect(renderer, &waveBg);
    
    renderText("WAVE ", WINDOW_WIDTH/2 - 50, 30, 2);
    renderNumber(wave, WINDOW_WIDTH/2 - 10, 30, 2);
    
    // Center top: Countdown timer with actual numbers
    float timeLeft = waveDuration - waveTimer;
    int seconds = (int)timeLeft;
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Semi-transparent black
    SDL_Rect timerBg = {WINDOW_WIDTH/2 - 60, 70, 120, 60};
    SDL_RenderFillRect(renderer, &timerBg);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
    SDL_RenderDrawRect(renderer, &timerBg);
    
    // Large timer numbers (centered)
    int timerDigits = std::to_string(seconds).length();
    int timerX = WINDOW_WIDTH/2 - (timerDigits * 12); // Center the number
    renderNumber(seconds, timerX, 85, 4); // Larger scale for timer
    
    // Experience bar (bottom of screen)
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // Dark green background
    SDL_Rect expBg = {0, WINDOW_HEIGHT - 15, WINDOW_WIDTH, 15};
    SDL_RenderFillRect(renderer, &expBg);
    
    // Experience progress (Brotato-style)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Bright green
    int expToNext = player->getExperienceToNextLevel();
    int currentExp = player->getExperience();
    
    // Calculate XP for current level using Brotato formula
    int expForCurrentLevel = 0;
    if (player->getLevel() > 1) {
        int currentLevel = player->getLevel();
        expForCurrentLevel = (currentLevel + 3 - 1) * (currentLevel + 3 - 1);
    }
    
    int expInCurrentLevel = currentExp - expForCurrentLevel;
    int expNeededForCurrentLevel = expToNext - expForCurrentLevel;
    
    if (expNeededForCurrentLevel > 0) {
        int expWidth = (expInCurrentLevel * WINDOW_WIDTH) / expNeededForCurrentLevel;
        SDL_Rect expBar = {0, WINDOW_HEIGHT - 15, expWidth, 15};
        SDL_RenderFillRect(renderer, &expBar);
    }
}

void Game::renderNumber(int number, int x, int y, int scale) {
    // Simple 5x7 bitmap font for digits 0-9
    static int digitPatterns[10][7] = {
        // 0
        {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
        // 1  
        {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
        // 2
        {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111},
        // 3
        {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110},
        // 4
        {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},
        // 5
        {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},
        // 6
        {0b01110, 0b10001, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110},
        // 7
        {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},
        // 8
        {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},
        // 9
        {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b10001, 0b01110}
    };
    
    std::string numStr = std::to_string(number);
    int currentX = x;
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White text
    
    for (char c : numStr) {
        if (c >= '0' && c <= '9') {
            int digit = c - '0';
            
            // Draw the digit
            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (digitPatterns[digit][row] & (1 << (4 - col))) {
                        SDL_Rect pixel = {currentX + col * scale, y + row * scale, scale, scale};
                        SDL_RenderFillRect(renderer, &pixel);
                    }
                }
            }
        }
        currentX += 6 * scale; // Move to next digit position
    }
}

void Game::renderText(const char* text, int x, int y, int scale) {
    // Simple letter patterns for common letters (5x7 bitmap)
    static std::map<char, std::vector<int>> letterPatterns = {
        {'A', {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
        {'B', {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}},
        {'C', {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}},
        {'D', {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}},
        {'E', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},
        {'F', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}},
        {'G', {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110}},
        {'H', {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
        {'I', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
        {'L', {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}},
        {'M', {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}},
        {'N', {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001}},
        {'O', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
        {'P', {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}},
        {'R', {0b11110, 0b10001, 0b10001, 0b11110, 0b10010, 0b10001, 0b10001}},
        {'S', {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}},
        {'T', {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
        {'U', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
        {'V', {0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100}},
        {'W', {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001}},
        {'.', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00100}},
        {'/', {0b00001, 0b00010, 0b00010, 0b00100, 0b00100, 0b01000, 0b10000}},
        {' ', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}}
    };
    
    int currentX = x;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White text
    
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if (c >= '0' && c <= '9') {
            // Handle numbers with renderNumber
            int digit = c - '0';
            renderNumber(digit, currentX, y, scale);
            currentX += 6 * scale;
        } else if (letterPatterns.find(c) != letterPatterns.end()) {
            // Draw the letter
            auto pattern = letterPatterns[c];
            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (pattern[row] & (1 << (4 - col))) {
                        SDL_Rect pixel = {currentX + col * scale, y + row * scale, scale, scale};
                        SDL_RenderFillRect(renderer, &pixel);
                    }
                }
            }
            currentX += 6 * scale; // Move to next character position
        } else {
            currentX += 6 * scale; // Skip unknown characters
        }
    }
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