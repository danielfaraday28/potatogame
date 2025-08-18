#include "Game.h"
#include "SlimeEnemy.h"
#include "PebblinEnemy.h"
#include "BossEnemy.h"
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
#define _USE_MATH_DEFINES
#include <cmath>

// State duration constants
const float Game::WAVE_COMPLETED_DURATION = 2.0f;
const float Game::SHOP_CLOSING_DURATION = 0.2f;  // Reduced from 1.0f
const float Game::WAVE_STARTING_DURATION = 0.8f;  // Reduced from 2.0f

Game::Game() : window(nullptr), renderer(nullptr), running(false), 
               timeSinceLastSpawn(0), score(0), wave(1), mousePos(0, 0),
               waveTimer(0), waveDuration(20.0f), waveActive(true), materialBag(0),
               defaultFont(nullptr), starTexture(nullptr), fKeyPressed(false), rKeyPressed(false), shopJustClosed(false), gameOverShown(false),
               currentState(GameState::WAVE_ACTIVE), stateTimer(0.0f), stateDuration(0.0f) {
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
    
    // Load particle star texture
    starTexture = IMG_LoadTexture(renderer, "assets/particles/star.png");
    if (!starTexture) {
        std::cout << "Failed to load star texture: " << IMG_GetError() << std::endl;
        std::cout << "Particle effects will be disabled" << std::endl;
        // Continue without particles (graceful degradation)
    } else {
        std::cout << "Loaded particle star texture" << std::endl;
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
    // Always update particles regardless of game state (even when dead or paused)
    updateParticles(deltaTime);
    
    // Check for game over condition
    if (player->getHealth() <= 0) {
        if (!gameOverShown) {
            // Add player death particle effect - red particles, medium amount, medium duration/speed
            createParticleBurst(player->getPosition(), 15, 120.0f, 1.0f, {255, 0, 0, 255}, 0.7f); // 30% smaller
            
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
    
    // Update state machine first
    updateState(deltaTime);
    
    // During transition states, allow limited gameplay
    if (currentState != GameState::WAVE_ACTIVE) {
        // Allow player movement and interaction during transitions
        player->update(deltaTime);
        
        // Update bullets (keep them moving)
        for (auto& bullet : bullets) {
            bullet->update(deltaTime);
        }
        
        // Update and allow collection of items
        updateExperienceCollection();
        updateMaterialCollection();
        
        // Update experience orbs and materials for visual movement
        for (auto& orb : experienceOrbs) {
            orb->update(deltaTime);
        }
        for (auto& material : materials) {
            material->update(deltaTime);
        }
        
        // Check collisions (player invulnerable during transitions)
        checkCollisions();
        
        // Clean up dead bullets
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](const std::unique_ptr<Bullet>& bullet) {
                return !bullet->isAlive();
            }), bullets.end());
        
        return;
    }
    
    // WAVE_ACTIVE state - full game logic
    if (waveActive) {
        waveTimer += deltaTime;
        
        // Check boss wave conditions
        if (isBossWaveIndex(wave) && !isBossWave && waveTimer < 1.0f) {
            startBossWave(wave);
            spawnIndicators.clear();
            timeSinceLastSpawn = 0;
        }
        
        // Update boss if present
        if (isBossWave && boss) {
            boss->update(deltaTime, player->getPosition(), bullets);
            
            // Check boss death
            if (boss->isDead()) {
                endBossWave(true);
                enterState(GameState::WAVE_COMPLETED);
                return;
            }
        }
        
        // Check wave completion
        if (waveTimer >= waveDuration) {
            enterState(GameState::WAVE_COMPLETED);
            return;
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
                // Add enemy death particle effect - enemy-specific color, small amount, low duration/speed
                SDL_Color enemyColor = getEnemyParticleColor(enemy->getEnemyType());
                createParticleBurst(enemy->getPosition(), 8, 80.0f, 0.6f, enemyColor, 0.49f); // 0.7f * 0.7f = ~30% smaller
                
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
    
    // Рендерим босса если он есть
    if (isBossWave && boss) {
        boss->render(renderer);
    }
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
    
    // Render particles (after background entities, before UI)
    renderParticles();
    
    renderUI();
    
    // Render shop on top if active
    shop->render(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Render transition state UI
    renderStateUI();
    
    // Render menu on top of everything if active
    menu->render(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    SDL_RenderPresent(renderer);
}

void Game::renderUI() {
    // === BROTATO-STYLE UI LAYOUT WITH BITMAP TEXT ===
    
    // Полоса здоровья босса (если есть)
    if (isBossWave && boss) {
        // Фон полосы HP босса
        SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
        SDL_Rect bossHpBg = {WINDOW_WIDTH/4, 50, WINDOW_WIDTH/2, 30};
        SDL_RenderFillRect(renderer, &bossHpBg);
        
        // Заполненная часть HP
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        int bossHpWidth = static_cast<int>((boss->getHealthPercent() * WINDOW_WIDTH/2));
        SDL_Rect bossHpBar = {WINDOW_WIDTH/4, 50, bossHpWidth, 30};
        SDL_RenderFillRect(renderer, &bossHpBar);
        
        // Белая рамка
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &bossHpBg);
        
        // Имя босса
        if (defaultFont) {
            SDL_Color white = {255, 255, 255, 255};
            renderTTFText(boss->getName(), WINDOW_WIDTH/2 - 50, 20, white, 24);
        } else {
            renderText(boss->getName(), WINDOW_WIDTH/2 - 50, 20, 2);
        }
    }
    
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

void Game::startBossWave(int waveIndex) {
    isBossWave = true;
    
    // Очищаем всех обычных врагов при старте босс-волны
    enemies.clear();
    
    // Reset player position to left bottom corner for boss waves
    if (player) {
        Vector2 newPlayerPos(100.0f, WINDOW_HEIGHT - 100.0f); // Left bottom corner with some margin
        player->setPosition(newPlayerPos);
    }
    
    // Создаем конфиг босса
    BossConfig config;
    // Разные имена боссов в зависимости от волны
    switch (waveIndex) {
        case 3:
            config.name = "MEGA SLIME";
            break;
        case 6:
            config.name = "DARK PEBBLIN";
            break;
        case 9:
            config.name = "MORTORHEAD PRIME";
            break;
        default:
            config.name = "ANCIENT ONE";
            break;
    }
    config.seed = static_cast<uint32_t>(waveIndex);
    
    // Настраиваем паттерны в зависимости от типа босса
    if (waveIndex == 3) { // MEGA SLIME - прыгучий босс с круговыми атаками
        config.weightsP1 = {
            {BossPattern::DASH_TO_PLAYER, 1.0f},  // Прыжки - основная атака
            {BossPattern::RADIAL_SHOTS, 0.8f},    // Часто стреляет по кругу
            {BossPattern::SUMMON_ADDS, 0.3f}      // Редко призывает миньонов
        };
        
        config.weightsP2 = {
            {BossPattern::DASH_TO_PLAYER, 1.2f},  // Еще больше прыжков
            {BossPattern::RADIAL_SHOTS, 1.0f},
            {BossPattern::SPIRAL_SHOTS, 0.8f},    // Новая атака
            {BossPattern::SUMMON_ADDS, 0.4f}
        };
        
        config.weightsEnrage = {
            {BossPattern::DASH_TO_PLAYER, 1.5f},  // Агрессивные прыжки
            {BossPattern::RADIAL_SHOTS, 1.2f},
            {BossPattern::SPIRAL_SHOTS, 1.0f},
            {BossPattern::BURST_AIM, 0.8f},       // Новая атака
            {BossPattern::SUMMON_ADDS, 0.5f}
        };
    }
    else if (waveIndex == 6) { // DARK PEBBLIN - призыватель с прицельной стрельбой
        config.weightsP1 = {
            {BossPattern::BURST_AIM, 1.0f},     // Основная атака - прицельная стрельба
            {BossPattern::SUMMON_ADDS, 0.8f},   // Часто призывает миньонов
            {BossPattern::RADIAL_SHOTS, 0.4f}   // Редкие защитные залпы
        };
        
        config.weightsP2 = {
            {BossPattern::BURST_AIM, 1.2f},     // Больше прицельной стрельбы
            {BossPattern::SUMMON_ADDS, 1.0f},   // Больше миньонов
            {BossPattern::SPIRAL_SHOTS, 0.7f},  // Добавляем спирали
            {BossPattern::DASH_TO_PLAYER, 0.3f} // Редкие рывки для позиционирования
        };
        
        config.weightsEnrage = {
            {BossPattern::BURST_AIM, 1.5f},     // Интенсивная стрельба
            {BossPattern::SUMMON_ADDS, 1.2f},   // Много миньонов
            {BossPattern::SPIRAL_SHOTS, 1.0f},  // Частые спирали
            {BossPattern::RADIAL_SHOTS, 0.8f},  // Защитные залпы
            {BossPattern::DASH_TO_PLAYER, 0.4f} // Редкие рывки
        };
    }
    else if (waveIndex == 9) { // MORTORHEAD PRIME - мастер спиральной стрельбы
        config.weightsP1 = {
            {BossPattern::SPIRAL_SHOTS, 1.0f},   // Основная атака - спирали
            {BossPattern::RADIAL_SHOTS, 0.7f},   // Дополнительные круговые атаки
            {BossPattern::DASH_TO_PLAYER, 0.4f}  // Редкие рывки
        };
        
        config.weightsP2 = {
            {BossPattern::SPIRAL_SHOTS, 1.2f},   // Больше спиралей
            {BossPattern::BURST_AIM, 0.9f},      // Добавляем прицельную стрельбу
            {BossPattern::RADIAL_SHOTS, 0.8f},   // Больше круговых атак
            {BossPattern::DASH_TO_PLAYER, 0.5f}  // Чаще рывки
        };
        
        config.weightsEnrage = {
            {BossPattern::SPIRAL_SHOTS, 1.5f},   // Интенсивные спирали
            {BossPattern::BURST_AIM, 1.2f},      // Много прицельной стрельбы
            {BossPattern::RADIAL_SHOTS, 1.0f},   // Частые круговые атаки
            {BossPattern::DASH_TO_PLAYER, 0.8f}, // Частые рывки
            {BossPattern::SUMMON_ADDS, 0.4f}     // Редкий призыв миньонов
        };
    }
    else { // ANCIENT ONE и другие - сбалансированный босс
        config.weightsP1 = {
            {BossPattern::RADIAL_SHOTS, 1.0f},
            {BossPattern::DASH_TO_PLAYER, 0.7f},
            {BossPattern::BURST_AIM, 0.5f}
        };
        
        config.weightsP2 = {
            {BossPattern::RADIAL_SHOTS, 1.1f},
            {BossPattern::SPIRAL_SHOTS, 1.0f},
            {BossPattern::DASH_TO_PLAYER, 0.8f},
            {BossPattern::BURST_AIM, 0.7f}
        };
        
        config.weightsEnrage = {
            {BossPattern::RADIAL_SHOTS, 1.2f},
            {BossPattern::SPIRAL_SHOTS, 1.2f},
            {BossPattern::BURST_AIM, 1.0f},
            {BossPattern::DASH_TO_PLAYER, 0.9f},
            {BossPattern::SUMMON_ADDS, 0.6f}
        };
    }
    
    // Создаем босса в центре верхней части экрана
    Vector2 spawnPos(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/3.f);
    SDL_Texture* bossTex = nullptr;
    
    // Выбираем текстуру босса в зависимости от волны
    const char* bossTexPath;
    switch (waveIndex) {
        case 3:
            bossTexPath = "assets/enemies/slime.png";
            break;
        case 6:
            bossTexPath = "assets/enemies/pebblin.png";
            break;
        case 9:
        default:
            bossTexPath = "assets/enemies/mortorhead.png";
            break;
    }
    SDL_Surface* surface = IMG_Load(bossTexPath);
    if (surface) {
        bossTex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    
    if (bossTex) {
        boss = std::make_unique<BossEnemy>(spawnPos, renderer, config);
    } else {
        std::cout << "Failed to load boss texture!" << std::endl;
    }
}

void Game::endBossWave(bool bossDefeated) {
    isBossWave = false;
    // Сохраняем позицию до сброса босса
    Vector2 rewardPos = boss ? boss->getPosition() : Vector2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f);
    boss.reset();
    
    if (bossDefeated) {
        // Дроп наград
        
        // Опыт (в 10 раз больше чем от обычного врага)
        for (int i = 0; i < 10; i++) {
            experienceOrbs.push_back(std::make_unique<ExperienceOrb>(rewardPos));
        }
        
        // Материалы (в 15 раз больше + бонус за волну)
        int materialCount = 15 + (wave / 3);
        for (int i = 0; i < materialCount; i++) {
            materials.push_back(std::make_unique<Material>(
                rewardPos,
                2 + (wave / 2),  // Увеличенное значение материалов
                3 + (wave / 2)   // Увеличенное значение опыта
            ));
        }
    }
}

void Game::spawnEnemies() {
    // Не спавним обычных врагов во время босс-волны
    if (isBossWave || isBossWaveIndex(wave)) return;
    
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
            // enemy bullets damage the player (only during active wave)
            if (currentState == GameState::WAVE_ACTIVE) {
                float d = bullet->getPosition().distance(player->getPosition());
                if (d < bullet->getRadius() + player->getRadius()) {
                    player->takeDamage(bullet->getDamage());
                    bullet->destroy();
                }
            }
            continue;
        }
        
        // Проверяем попадание по боссу
        if (isBossWave && boss && boss->isAlive()) {
            float distance = bullet->getPosition().distance(boss->getPosition());
            if (distance < bullet->getRadius() + boss->getRadius()) {
                bullet->destroy();
                boss->takeDamage(bullet->getDamage());
            }
        }
        
        // Проверяем попадание по обычным врагам
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
    
    // Проверяем коллизию с боссом
    if (isBossWave && boss && boss->isAlive()) {
        float bossDistance = player->getPosition().distance(boss->getPosition());
        if (bossDistance < player->getRadius() + boss->getRadius()) {
            player->takeDamage(boss->getDamage());
            // Босса не уничтожаем при столкновении, только наносим урон игроку
        }
    }

    // Проверяем коллизии с обычными врагами
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
                        // Add enemy death particle effect - enemy-specific color, small amount, low duration/speed
                        SDL_Color enemyColor = getEnemyParticleColor(enemy->getEnemyType());
                        createParticleBurst(enemy->getPosition(), 8, 80.0f, 0.6f, enemyColor, 0.49f); // 0.7f * 0.7f = ~30% smaller
                        enemy->hit();
                        enemy->destroy();
                    }
                }
            }
        }
    }
}

void Game::cleanup() {
    if (starTexture) {
        SDL_DestroyTexture(starTexture);
        starTexture = nullptr;
    }
    
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
    
    // Reset state machine
    currentState = GameState::WAVE_ACTIVE;
    stateTimer = 0.0f;
    stateDuration = 0.0f;
    
    // Сбрасываем состояние босс-волны
    isBossWave = false;
    boss.reset();
    
    // Clear all game entities
    enemies.clear();
    bullets.clear();
    experienceOrbs.clear();
    materials.clear();
    bombs.clear();
    particles.clear();
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

void Game::enterState(GameState newState) {
    currentState = newState;
    stateTimer = 0.0f;
    
    switch (newState) {
        case GameState::WAVE_COMPLETED:
            stateDuration = WAVE_COMPLETED_DURATION;
            // Add enemy-specific colored particle effects for enemies disappearing at wave end
            for (auto& enemy : enemies) {
                if (enemy->isAlive()) {
                    SDL_Color enemyColor = getEnemyParticleColor(enemy->getEnemyType());
                    createParticleBurst(enemy->getPosition(), 15, 120.0f, 1.0f, enemyColor, 0.63f); // 0.9f * 0.7f = ~30% smaller
                }
            }
            // Clear all enemies from the map
            enemies.clear();
            // Keep bullets on screen - don't clear them
            // Clear spawn indicators to prevent late spawns
            spawnIndicators.clear();
            // Distribute bagged materials to player
            if (materialBag > 0) {
                player->gainMaterials(materialBag);
                std::cout << "Collected " << materialBag << " materials from bag!" << std::endl;
                materialBag = 0;
            }
            // Increment wave number and prepare for next wave
            wave++;
            waveTimer = 0;
            // Increase wave duration for next wave
            if (waveDuration < 60.0f) {
                waveDuration += 5.0f;
                if (waveDuration > 60.0f) waveDuration = 60.0f;
            }
            std::cout << "Wave " << wave - 1 << " completed!" << std::endl;
            break;
            
        case GameState::SHOP_ACTIVE:
            stateDuration = 0.0f; // Indefinite until player closes shop
            shop->openShop(wave);
            std::cout << "Shop opened after wave " << wave << std::endl;
            break;
            
        case GameState::SHOP_CLOSING:
            stateDuration = SHOP_CLOSING_DURATION;
            shop->closeShop();
            break;
            
        case GameState::WAVE_STARTING:
            stateDuration = WAVE_STARTING_DURATION;
            // Reset spawn timer
            timeSinceLastSpawn = 0;
            // Reload all weapons at wave start
            if (player) {
                player->reloadAllWeapons();
            }
            std::cout << "Wave " << wave << " starting..." << std::endl;
            break;
            
        case GameState::WAVE_ACTIVE:
            stateDuration = 0.0f; // Indefinite until wave completes
            std::cout << "Wave " << wave << " active!" << std::endl;
            break;
    }
}

void Game::updateState(float deltaTime) {
    stateTimer += deltaTime;
    
    // Check for automatic state transitions
    switch (currentState) {
        case GameState::WAVE_COMPLETED:
            if (stateTimer >= stateDuration) {
                enterState(GameState::SHOP_ACTIVE);
            }
            break;
            
        case GameState::SHOP_ACTIVE:
            // Transition handled when shop is closed
            if (!shop->isShopActive()) {
                enterState(GameState::SHOP_CLOSING);
            }
            break;
            
        case GameState::SHOP_CLOSING:
            if (stateTimer >= stateDuration) {
                enterState(GameState::WAVE_STARTING);
            }
            break;
            
        case GameState::WAVE_STARTING:
            if (stateTimer >= stateDuration) {
                enterState(GameState::WAVE_ACTIVE);
            }
            break;
            
        case GameState::WAVE_ACTIVE:
            // Transition handled when wave timer completes
            break;
    }
}

void Game::renderStateUI() {
    if (currentState == GameState::WAVE_ACTIVE || currentState == GameState::SHOP_ACTIVE) {
        return; // No special state UI during active gameplay or shop
    }
    
    // No black overlay - just render text messages over the game field
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255};
    int centerX = WINDOW_WIDTH / 2;
    int centerY = WINDOW_HEIGHT / 2;
    
    switch (currentState) {
        case GameState::WAVE_COMPLETED: {
            std::string message = "WAVE " + std::to_string(wave - 1) + " COMPLETED!";
            // Draw text with black outline for better visibility
            renderTTFText(message.c_str(), centerX - 149, centerY - 19, black, 36); // Shadow
            renderTTFText(message.c_str(), centerX - 150, centerY - 20, white, 36); // Main text
            break;
        }
            
        case GameState::SHOP_CLOSING:
            // No message needed during shop closing
            break;
            
        case GameState::WAVE_STARTING: {
            std::string message;
            if (isBossWaveIndex(wave)) {
                // For boss waves, show the boss name
                if (boss) {
                    message = boss->getName();
                } else {
                    message = "BOSS WAVE STARTING...";
                }
            } else {
                message = "WAVE " + std::to_string(wave) + " STARTING...";
            }
            renderTTFText(message.c_str(), centerX - 149, centerY - 9, black, 32); // Shadow
            renderTTFText(message.c_str(), centerX - 150, centerY - 10, white, 32); // Main text
            break;
        }
        
        case GameState::SHOP_ACTIVE:
        case GameState::WAVE_ACTIVE:
            // No overlay message for these states
            break;
    }
}

// Particle system implementation
SDL_Color Game::getEnemyParticleColor(EnemyType enemyType) {
    switch (enemyType) {
        case EnemyType::BASE:
            return {100, 150, 255, 255}; // Blue
        case EnemyType::PEBBLIN:
            return {139, 119, 101, 255}; // Stone color (brownish gray)
        case EnemyType::SLIME:
            return {60, 179, 113, 255};  // Swamp green
        default:
            return {255, 0, 0, 255};     // Default red fallback
    }
}

void Game::updateParticles(float deltaTime) {
    for (auto& particle : particles) {
        particle->update(deltaTime);
    }
}

void Game::renderParticles() {
    if (!starTexture) return;
    
    for (auto& particle : particles) {
        if (particle->isAlive()) {
            particle->render(renderer, starTexture);
        }
    }
}

void Game::createParticleBurst(Vector2 position, int particleCount, float particleSpeed,
                              float normalDuration, SDL_Color color, float scale) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> speedVariation(0.8f, 1.2f);
    
    for (int i = 0; i < particleCount; ++i) {
        // Random direction (360 degrees)
        float angle = angleDist(gen);
        float speed = particleSpeed * speedVariation(gen);
        
        Vector2 velocity(
            std::cos(angle) * speed,
            std::sin(angle) * speed
        );
        
        particles.push_back(std::make_unique<Particle>(
            position, velocity, normalDuration, color, scale
        ));
    }
}

void Game::createExplosionEffect(Vector2 position, SDL_Color color) {
    // Large burst with high-speed particles
    createParticleBurst(position, 50, 200.0f, 1.5f, color, 1.0f);
}

void Game::createDeathEffect(Vector2 position, SDL_Color color) {
    // Medium burst with medium-speed particles
    createParticleBurst(position, 20, 150.0f, 1.0f, color, 0.8f);
}

void Game::createImpactEffect(Vector2 position, SDL_Color color) {
    // Small burst with low-speed particles
    createParticleBurst(position, 15, 100.0f, 0.5f, color, 0.6f);
}