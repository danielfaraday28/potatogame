#include "Game.h"
#include "SlimeEnemy.h"
#include "PebblinEnemy.h"
#include <cmath>
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
               waveTimer(0), waveDuration(20.0f), waveActive(true), materialBag(0),
               defaultFont(nullptr), fKeyPressed(false), rKeyPressed(false), shopJustClosed(false), gameOverShown(false) {
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
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    
    window = SDL_CreateWindow("Brotato MVP", 
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
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
    
    // Initialize player with renderer for sprite loading
    player->initialize(renderer);
    
    // Initialize player weapons with renderer for sprite loading
    player->initializeWeapons(renderer);
    
    shop = std::make_unique<Shop>();
    shop->setGame(this);
    shop->loadAssets(renderer);
    
    menu = std::make_unique<Menu>();
    menu->setGame(this);
    
    // Try to load fonts in order of preference
    const char* fontPaths[] = {
        "assets/fonts/default.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/consola.ttf"
    };
    
    defaultFont = nullptr;
    for (const char* fontPath : fontPaths) {
        defaultFont = TTF_OpenFont(fontPath, 16);
        if (defaultFont) {
            std::cout << "Loaded font: " << fontPath << std::endl;
            break;
        }
    }
    
    if (!defaultFont) {
        std::cout << "No TTF font available - using bitmap fallback" << std::endl;
        // Continue without TTF font - will fall back to bitmap rendering
    }
    
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
    
    // Ensure menu and shop are not both active
    if (menu->isMenuActive() && shop->isShopActive()) {
        shop->closeShop();
    }
    
    // Handle menu input if menu is active
    if (menu->isMenuActive()) {
        menu->handleInput(keyState);
        
        // Handle mouse input for menu
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
        bool mousePressed = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        menu->handleMouseInput(mouseX, mouseY, mousePressed);
    }
    // Handle shop input if shop is active
    else if (shop->isShopActive()) {
        bool shopWasActive = shop->isShopActive();
        shop->handleInput(keyState, *player);
        
        // Check if shop was closed by ESC key
        if (shopWasActive && !shop->isShopActive() && keyState[SDL_SCANCODE_ESCAPE]) {
            shopJustClosed = true;
        }
        
        // Handle mouse input for shop
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
        bool mousePressed = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        shop->handleMouseInput(mouseX, mouseY, mousePressed, *player);
    } 
    // Handle game input (including ESC for pause)
    else {
        // ESC key handling for pause menu (only when not in shop)
        static bool escKeyPressed = false;
        
        // Don't process ESC if shop just closed this frame
        if (shopJustClosed) {
            shopJustClosed = false; // Reset flag
            escKeyPressed = keyState[SDL_SCANCODE_ESCAPE]; // Set ESC state to current key state
        }
        else if (keyState[SDL_SCANCODE_ESCAPE] && !escKeyPressed) {
            escKeyPressed = true;
            showPauseMenu();
            // Tell pause menu that ESC key is currently pressed to avoid immediate close
            menu->setEscKeyPressed(true);
        } else if (!keyState[SDL_SCANCODE_ESCAPE]) {
            escKeyPressed = false;
        }
        
        player->handleInput(keyState);
        handleItemInput(keyState);
        
        // Update player's shoot direction to point towards mouse
        player->updateShootDirection(mousePos);
    }
}

void Game::update(float deltaTime) {
    // Check for game over condition
    if (player->getHealth() <= 0) {
        if (!gameOverShown) {
            // Ensure shop is closed if player dies
            if (shop->isShopActive()) {
                shop->closeShop();
            }
            showGameOverMenu();
            gameOverShown = true; // Set flag to prevent repeated calls
        }
        return; // Don't update game logic when game is over
    }
    
    // Don't update game if menu is active (pause functionality)
    if (menu->isMenuActive()) {
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
    
    // Update bombs
    updateBombs(deltaTime);
    checkBombExplosions();
    
    for (auto& bullet : bullets) {
        bullet->update(deltaTime);
    }
    
    for (auto& enemy : enemies) {
        enemy->update(deltaTime, player->getPosition(), bullets);
    }

    updateSpawnIndicators(deltaTime);
    
    for (auto& orb : experienceOrbs) {
        orb->update(deltaTime);
    }
    
    for (auto& material : materials) {
        material->update(deltaTime);
    }
    
    spawnEnemies();
    checkCollisions();
    checkMeleeAttacks();
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
    SDL_SetRenderDrawColor(renderer, 120, 110, 100, 255); // Light brown/tan background for better visibility
    SDL_RenderClear(renderer);
    
    player->render(renderer);
    player->renderWeapons(renderer);
    
    for (auto& bullet : bullets) {
        bullet->render(renderer);
    }
    
    for (auto& enemy : enemies) {
        enemy->render(renderer);
    }

    // Spawn indicators on top of background but beneath UI
    renderSpawnIndicators();
    
    for (auto& orb : experienceOrbs) {
        orb->render(renderer);
    }
    
    for (auto& material : materials) {
        material->render(renderer);
    }
    
    // Render bombs
    renderBombs();
    
    renderUI();
    
    // Render shop on top if active
    shop->render(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Render menu on top of everything if active
    menu->render(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
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
    
    // Item slots with keybind hints
    int itemSlotSize = 40;
    int itemSpacing = 10;
    int itemStartX = 20;
    int itemStartY = 200;
    
    // Draw item slots
    for (int i = 0; i < player->getItemCount(); i++) {
        SDL_Rect slotRect = {itemStartX + i * (itemSlotSize + itemSpacing), itemStartY, itemSlotSize, itemSlotSize};
        
        // Slot background
        SDL_SetRenderDrawColor(renderer, 40, 45, 50, 255);
        SDL_RenderFillRect(renderer, &slotRect);
        
        // Border
        SDL_SetRenderDrawColor(renderer, 100, 110, 120, 255);
        SDL_RenderDrawRect(renderer, &slotRect);
        
        // Draw item icon (placeholder for now)
        const Item* item = player->getItem(i);
        if (item) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_Rect iconRect = {slotRect.x + 8, slotRect.y + 8, slotRect.w - 16, slotRect.h - 16};
            SDL_RenderFillRect(renderer, &iconRect);
            
            // Draw keybind hint
            SDL_Color white = {255, 255, 255, 255};
            if (item->getType() == ItemType::HEALING_BOX) {
                renderTTFText("F", slotRect.x + 4, slotRect.y + slotRect.h + 4, white, 12);
            } else if (item->getType() == ItemType::MASS_BOMB) {
                renderTTFText("R", slotRect.x + 4, slotRect.y + slotRect.h + 4, white, 12);
            }
        }
    }
    
    // Center top: Wave number with TTF text
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Semi-transparent black
    SDL_Rect waveBg = {WINDOW_WIDTH/2 - 80, 20, 160, 40};
    SDL_RenderFillRect(renderer, &waveBg);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
    SDL_RenderDrawRect(renderer, &waveBg);
    
    // Use TTF font for cleaner wave display, fallback to bitmap if TTF fails
    if (defaultFont) {
        SDL_Color waveColor = {255, 255, 255, 255};
        std::string waveText = "WAVE " + std::to_string(wave);
        renderTTFText(waveText.c_str(), WINDOW_WIDTH/2 - 40, 28, waveColor, 18);
    } else {
        // Fallback to bitmap rendering with better spacing
        renderText("WAVE", WINDOW_WIDTH/2 - 50, 30, 2);
        renderNumber(wave, WINDOW_WIDTH/2 + 10, 30, 2);
    }
    
    // Center top: Countdown timer with actual numbers
    float timeLeft = waveDuration - waveTimer;
    int seconds = (int)timeLeft;
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Semi-transparent black
    SDL_Rect timerBg = {WINDOW_WIDTH/2 - 60, 70, 120, 60};
    SDL_RenderFillRect(renderer, &timerBg);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
    SDL_RenderDrawRect(renderer, &timerBg);
    
    // Large timer numbers using TTF (centered), fallback to bitmap
    if (defaultFont) {
        SDL_Color timerColor = {255, 255, 255, 255};
        std::string timerText = std::to_string(seconds);
        renderTTFText(timerText.c_str(), WINDOW_WIDTH/2 - 15, 80, timerColor, 28);
    } else {
        // Fallback to bitmap rendering
        int timerDigits = std::to_string(seconds).length();
        int timerX = WINDOW_WIDTH/2 - (timerDigits * 12);
        renderNumber(seconds, timerX, 85, 4);
    }
    
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
        {'J', {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100}},
        {'K', {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}},
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
        {'X', {0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b01010, 0b10001}},
        {'Y', {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}},
        {'Z', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}},
        {'.', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00100}},
        {'/', {0b00001, 0b00010, 0b00010, 0b00100, 0b00100, 0b01000, 0b10000}},
        {'|', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
        {'=', {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000}},
        {':', {0b00000, 0b00100, 0b00000, 0b00000, 0b00000, 0b00100, 0b00000}},
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

void Game::renderTTFText(const char* text, int x, int y, SDL_Color color, int fontSize) {
    if (!defaultFont) return; // Fall back to bitmap rendering if no TTF font
    
    // Create surface from text
    SDL_Surface* textSurface = TTF_RenderText_Blended(defaultFont, text, color);
    if (!textSurface) {
        std::cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    // Create texture from surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cout << "Unable to create texture from text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }
    
    // Render texture
    SDL_Rect destRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);
    
    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Game::spawnEnemies() {
    timeSinceLastSpawn += 0.016f;
    
    float spawnRate = 1.0f - (wave * 0.1f);
    if (spawnRate < 0.2f) spawnRate = 0.2f;
    
    if (timeSinceLastSpawn >= spawnRate) {
        timeSinceLastSpawn = 0;
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> xdist(0.0f, static_cast<float>(WINDOW_WIDTH));
        std::uniform_real_distribution<float> ydist(0.0f, static_cast<float>(WINDOW_HEIGHT));
        std::uniform_real_distribution<float> slimeChance(0.0f, 1.0f);

        Vector2 spawnPos(xdist(gen), ydist(gen));

        // Queue a flashing red X indicator before actual spawn
        float telegraphDuration = spawnTelegraphSeconds; // configurable
        
        // Determine enemy type based on wave
        EnemySpawnType enemyType = EnemySpawnType::BASE;
        if (wave >= 2) {
            // From wave 2: 40% slime, 40% pebblin, 20% base
            float typeRoll = slimeChance(gen);
            if (typeRoll < 0.4f) {
                enemyType = EnemySpawnType::SLIME;
            } else if (typeRoll < 0.8f) {
                enemyType = EnemySpawnType::PEBBLIN;
            }
        } else {
            // Wave 1: 50% slime, 50% base (no pebblin yet)
            if (slimeChance(gen) < 0.5f) {
                enemyType = EnemySpawnType::SLIME;
            }
        }
        
        spawnIndicators.emplace_back(spawnPos, telegraphDuration, enemyType);
    }
}

void Game::updateSpawnIndicators(float deltaTime) {
    for (auto& indicator : spawnIndicators) {
        indicator.elapsed += deltaTime;
    }

    // Spawn enemies for completed indicators, then remove them
    std::vector<SpawnIndicator> remaining;
    remaining.reserve(spawnIndicators.size());
    for (auto& indicator : spawnIndicators) {
        if (indicator.isComplete()) {
            switch (indicator.enemyType) {
                case EnemySpawnType::SLIME:
                    enemies.push_back(CreateSlimeEnemy(indicator.position, renderer));
                    break;
                case EnemySpawnType::PEBBLIN:
                    enemies.push_back(CreatePebblinEnemy(indicator.position, renderer));
                    break;
                case EnemySpawnType::BASE:
                default:
                    enemies.push_back(std::make_unique<Enemy>(indicator.position, renderer));
                    break;
            }
        } else {
            remaining.push_back(indicator);
        }
    }
    spawnIndicators.swap(remaining);
}

void Game::renderSpawnIndicators() {
    for (auto& indicator : spawnIndicators) {
        // Flashing red X effect, fades in/out over duration
        float t = indicator.elapsed;
        float flash = fmodf(t * 6.0f, 2.0f) < 1.0f ? 255.0f : 80.0f; // blink ~3 Hz
        Uint8 alpha = static_cast<Uint8>(flash);
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, alpha);

        int size = 14; // cross arm length
        int cx = static_cast<int>(indicator.position.x);
        int cy = static_cast<int>(indicator.position.y);
        // 4 short lines to make an X
        for (int i = -size; i <= size; ++i) {
            SDL_RenderDrawPoint(renderer, cx + i, cy + i);
            SDL_RenderDrawPoint(renderer, cx + i, cy - i);
        }
    }
}

void Game::checkCollisions() {
    for (auto& bullet : bullets) {
        if (!bullet->isAlive()) continue;
        if (bullet->isEnemyOwned()) {
            // enemy bullets damage the player
            float d = bullet->getPosition().distance(player->getPosition());
            if (d < bullet->getRadius() + player->getRadius()) {
                player->takeDamage(bullet->getDamage());
                bullet->destroy();
            }
            continue;
        }
        for (auto& enemy : enemies) {
            if (bullet->isAlive() && enemy->isAlive()) {
                float distance = bullet->getPosition().distance(enemy->getPosition());
                if (distance < bullet->getRadius() + enemy->getRadius()) {
                    bullet->destroy();
                    enemy->hit();
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

void Game::checkMeleeAttacks() {
    // Check if player has any melee weapons that are currently attacking
    for (int i = 0; i < player->getWeaponCount(); i++) {
        const Weapon* weapon = player->getWeapon(i);
        if (weapon && weapon->isMeleeWeapon() && weapon->isAttacking()) {
            // Only damage during the peak of the attack (when weapon is most extended)
            float attackProgress = weapon->getAttackProgress();
            if (attackProgress < 0.4f || attackProgress > 0.8f) {
                continue; // Only damage during middle 40% of animation
            }
            
            // Calculate weapon tip position
            Vector2 weaponTip = weapon->getWeaponTipPosition(player->getPosition(), player->getShootDirection());
            float damageRadius = 25.0f; // Smaller damage radius at weapon tip
            int meleeDamage = weapon->calculateDamage(*player);
            
            // Check for critical hit
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> critRoll(0.0f, 1.0f);
            
            if (critRoll(gen) < weapon->getStats().critChance) {
                meleeDamage = (int)(meleeDamage * weapon->getStats().critMultiplier);
            }
            
            // Damage all enemies within damage radius of weapon tip (Brotato-style infinite pierce)
            for (auto& enemy : enemies) {
                if (enemy->isAlive()) {
                    float distance = weaponTip.distance(enemy->getPosition());
                    if (distance <= damageRadius + enemy->getRadius()) {
                        enemy->hit();
                        enemy->destroy();
                        
                        // Create experience orb at enemy position
                        experienceOrbs.push_back(std::make_unique<ExperienceOrb>(enemy->getPosition()));
                        
                        // Chance to drop materials
                        static std::random_device matRd;
                        static std::mt19937 matGen(matRd());
                        std::uniform_real_distribution<float> matChance(0.0f, 1.0f);
                        
                        if (matChance(matGen) < getMaterialDropChance()) {
                            materials.push_back(std::make_unique<Material>(enemy->getPosition()));
                        }
                    }
                }
            }
        }
    }
}

void Game::handleItemInput(const Uint8* keyState) {
    // F key for healing box
    if (keyState[SDL_SCANCODE_F]) {
        if (!fKeyPressed) {
            // Find first healing box
            for (int i = 0; i < player->getItemCount(); i++) {
                if (const Item* item = player->getItem(i)) {
                    if (item->getType() == ItemType::HEALING_BOX) {
                        player->useItem(i, *this);
                        break;
                    }
                }
            }
        }
        fKeyPressed = true;
    } else {
        fKeyPressed = false;
    }
    
    // R key for bomb
    if (keyState[SDL_SCANCODE_R]) {
        if (!rKeyPressed) {
            // Find first bomb
            for (int i = 0; i < player->getItemCount(); i++) {
                if (const Item* item = player->getItem(i)) {
                    if (item->getType() == ItemType::MASS_BOMB) {
                        player->useItem(i, *this);
                        break;
                    }
                }
            }
        }
        rKeyPressed = true;
    } else {
        rKeyPressed = false;
    }
}

void Game::addBomb(Vector2 position, float timer, float radius, int damage) {
    bombs.push_back(std::make_unique<Bomb>(position, timer, radius, damage));
}

void Game::updateBombs(float deltaTime) {
    for (auto& bomb : bombs) {
        bomb->update(deltaTime);
    }
    
    // Remove dead bombs
    bombs.erase(std::remove_if(bombs.begin(), bombs.end(),
        [](const std::unique_ptr<Bomb>& bomb) {
            return !bomb->isAlive();
        }), bombs.end());
}

void Game::renderBombs() {
    for (auto& bomb : bombs) {
        bomb->render(renderer);
    }
}

void Game::checkBombExplosions() {
    for (auto& bomb : bombs) {
        if (bomb->isExploded()) {
            Vector2 bombPos = bomb->getPosition();
            float bombRadius = bomb->getRadius();
            int bombDamage = bomb->getDamage();
            
            // Damage enemies in radius
            for (auto& enemy : enemies) {
                if (enemy->isAlive()) {
                    float distance = bombPos.distance(enemy->getPosition());
                    if (distance <= bombRadius + enemy->getRadius()) {
                        enemy->hit();
                        enemy->destroy();
                    }
                }
            }
        }
    }
}

void Game::cleanup() {
    if (defaultFont) {
        TTF_CloseFont(defaultFont);
        defaultFont = nullptr;
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Menu management methods
void Game::showPauseMenu() {
    menu->showMenu(MenuType::PAUSE);
}

void Game::showGameOverMenu() {
    // Ensure shop is closed when showing game over menu
    if (shop->isShopActive()) {
        shop->closeShop();
    }
    menu->showMenu(MenuType::GAME_OVER);
}

void Game::restartGame() {
    // Full game reset
    wave = 1;
    waveTimer = 0;
    waveDuration = 20.0f;
    waveActive = true;
    materialBag = 0;
    score = 0;
    timeSinceLastSpawn = 0;
    gameOverShown = false; // Reset game over flag
    
    // Clear all game entities
    enemies.clear();
    bullets.clear();
    experienceOrbs.clear();
    materials.clear();
    bombs.clear();
    spawnIndicators.clear();
    
    // Reset player to starting state
    player = std::make_unique<Player>(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    player->initialize(renderer);
    player->initializeWeapons(renderer);
    
    // Close shop and menu
    shop->closeShop();
    menu->hideMenu();
}

void Game::exitGame() {
    running = false;
}

bool Game::isGamePaused() const {
    return menu->isMenuActive();
}