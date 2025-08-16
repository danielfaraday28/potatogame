#include "Menu.h"
#include "Game.h"
#include <iostream>
#include <string>

Menu::Menu() 
    : active(false), menuType(MenuType::PAUSE), selectedButton(0), hoveredButton(-1), lastMousePressed(false),
      escKeyPressed(false), enterKeyPressed(false), upKeyPressed(false), downKeyPressed(false),
      justShown(false), gameRef(nullptr) {
}

Menu::~Menu() {
}



void Menu::showMenu(MenuType type) {
    active = true;
    menuType = type;
    selectedButton = (type == MenuType::PAUSE) ? 0 : 1; // Start with "Continue" for pause, "Restart" for game over
    hoveredButton = -1;
    
    // Reset key states
    enterKeyPressed = false;
    upKeyPressed = false;
    downKeyPressed = false;
    
    // Only prevent ESC processing on first frame for PAUSE menu (which is triggered by ESC)
    // Game over menu should allow immediate input since it's not triggered by ESC
    if (type == MenuType::PAUSE) {
        justShown = true; // Prevent ESC processing on first frame for pause menu
        escKeyPressed = false; // Will be set by Game if ESC is currently held
    } else {
        justShown = false; // Game over menu allows immediate input
        escKeyPressed = false; // Reset ESC state for game over menu
    }
}

void Menu::hideMenu() {
    active = false;
}

void Menu::render(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    if (!active) return;
    
    // Get actual renderer size (handles fullscreen desktop mode correctly)
    int actualWidth, actualHeight;
    SDL_GetRendererOutputSize(renderer, &actualWidth, &actualHeight);
    
    // Render blur background
    renderBlurBackground(renderer, actualWidth, actualHeight);
    
    // Calculate menu position (centered) using actual dimensions
    int menuX = (actualWidth - MENU_WIDTH) / 2;
    int menuY = (actualHeight - MENU_HEIGHT) / 2;
    
    // Draw menu background with colored rectangles
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 25, 30, 40, 230);
    SDL_Rect bgRect = {menuX, menuY, MENU_WIDTH, MENU_HEIGHT};
    SDL_RenderFillRect(renderer, &bgRect);
    SDL_SetRenderDrawColor(renderer, 80, 120, 180, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    // Render title
    SDL_Color white = {255, 255, 255, 255};
    std::string title = getMenuTitle();
    renderTTFText(renderer, title.c_str(), menuX + MENU_WIDTH / 2 - (title.length() * 6), menuY + 40, white, 32);
    
    // Calculate button positions
    int buttonStartY = menuY + 120;
    int buttonX = menuX + (MENU_WIDTH - BUTTON_WIDTH) / 2;
    
    // Render buttons
    for (int i = 0; i < TOTAL_BUTTONS; i++) {
        MenuButton button = static_cast<MenuButton>(i);
        int buttonY = buttonStartY + i * (BUTTON_HEIGHT + BUTTON_SPACING);
        
        bool highlighted = (i == selectedButton) || (i == hoveredButton);
        bool disabled = !isButtonEnabled(button);
        
        renderButton(renderer, button, buttonX, buttonY, highlighted, disabled);
    }
    
    // Render controls help text
    SDL_Color helpTextColor = {160, 170, 185, 255};
    renderTTFText(renderer, "Use arrow keys and ENTER, or click with mouse", 
                  menuX + 20, menuY + MENU_HEIGHT - 40, helpTextColor, 14);
}

void Menu::renderBlurBackground(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    // Create a semi-transparent dark overlay with subtle color tint
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 15, 25, 160); // Dark blue-tinted overlay
    SDL_Rect fullScreen = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &fullScreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Menu::renderButton(SDL_Renderer* renderer, MenuButton button, int x, int y, bool highlighted, bool disabled) {
    // Draw button background with colored rectangles
    SDL_Rect buttonRect = {x, y, BUTTON_WIDTH, BUTTON_HEIGHT};
    
    if (disabled) {
        SDL_SetRenderDrawColor(renderer, 45, 45, 50, 255);
    } else if (highlighted) {
        SDL_SetRenderDrawColor(renderer, 70, 120, 200, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 55, 65, 80, 255);
    }
    SDL_RenderFillRect(renderer, &buttonRect);
    
    // Draw button border
    if (disabled) {
        SDL_SetRenderDrawColor(renderer, 80, 80, 85, 255);
    } else if (highlighted) {
        SDL_SetRenderDrawColor(renderer, 120, 160, 220, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 120, 150, 255);
    }
    SDL_RenderDrawRect(renderer, &buttonRect);
    
    // Draw button text
    SDL_Color textColor;
    if (disabled) {
        textColor = {120, 120, 125, 255}; // Light gray for disabled
    } else if (highlighted) {
        textColor = {255, 255, 255, 255}; // Bright white for highlighted
    } else {
        textColor = {220, 220, 230, 255}; // Slightly dimmed white for normal
    }
    
    std::string buttonText = getButtonText(button);
    int textX = x + (BUTTON_WIDTH - buttonText.length() * 8) / 2; // Rough centering
    int textY = y + (BUTTON_HEIGHT - 16) / 2;
    renderTTFText(renderer, buttonText.c_str(), textX, textY, textColor, 18);
}

void Menu::renderTTFText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, int fontSize) {
    if (gameRef) {
        gameRef->renderTTFText(text, x, y, color, fontSize);
    }
}

bool Menu::isButtonEnabled(MenuButton button) const {
    // Continue button is only enabled in pause menu
    if (button == MenuButton::CONTINUE) {
        return menuType == MenuType::PAUSE;
    }
    // Restart and Exit are always enabled
    return true;
}

std::string Menu::getButtonText(MenuButton button) const {
    switch (button) {
        case MenuButton::CONTINUE: return "CONTINUE";
        case MenuButton::RESTART: return "RESTART";
        case MenuButton::EXIT: return "EXIT";
    }
    return "";
}

std::string Menu::getMenuTitle() const {
    switch (menuType) {
        case MenuType::PAUSE: return "PAUSE";
        case MenuType::GAME_OVER: return "GAME OVER";
    }
    return "";
}

void Menu::handleInput(const Uint8* keyState) {
    if (!active) return;
    
    // Handle ESC key - only trigger action on key press, not hold
    // Skip ESC processing on first frame if menu was just shown to prevent race condition
    if (keyState[SDL_SCANCODE_ESCAPE]) {
        if (!escKeyPressed && !justShown) {
            // ESC key was just pressed and we're not on the first frame
            escKeyPressed = true;
            if (menuType == MenuType::PAUSE) {
                continueGame();
            } else if (menuType == MenuType::GAME_OVER) {
                exitGame();
            }
        }
        // If escKeyPressed is already true, ESC is being held - ignore
    } else {
        // ESC key was released - now ready to process next press
        escKeyPressed = false;
    }
    
    // Clear justShown flag after first frame, but allow other input processing
    if (justShown) {
        justShown = false;
        // Don't return here - allow keyboard navigation to work immediately
    }
    
    // Handle Enter key (activate selected button)
    if (keyState[SDL_SCANCODE_RETURN] && !enterKeyPressed) {
        enterKeyPressed = true;
        MenuButton button = static_cast<MenuButton>(selectedButton);
        if (isButtonEnabled(button)) {
            switch (button) {
                case MenuButton::CONTINUE: continueGame(); break;
                case MenuButton::RESTART: restartGame(); break;
                case MenuButton::EXIT: exitGame(); break;
            }
        }
    } else if (!keyState[SDL_SCANCODE_RETURN]) {
        enterKeyPressed = false;
    }
    
    // Handle up arrow
    if (keyState[SDL_SCANCODE_UP] && !upKeyPressed) {
        upKeyPressed = true;
        do {
            selectedButton--;
            if (selectedButton < 0) selectedButton = TOTAL_BUTTONS - 1;
        } while (!isButtonEnabled(static_cast<MenuButton>(selectedButton)));
    } else if (!keyState[SDL_SCANCODE_UP]) {
        upKeyPressed = false;
    }
    
    // Handle down arrow
    if (keyState[SDL_SCANCODE_DOWN] && !downKeyPressed) {
        downKeyPressed = true;
        do {
            selectedButton++;
            if (selectedButton >= TOTAL_BUTTONS) selectedButton = 0;
        } while (!isButtonEnabled(static_cast<MenuButton>(selectedButton)));
    } else if (!keyState[SDL_SCANCODE_DOWN]) {
        downKeyPressed = false;
    }
}

void Menu::handleMouseInput(int mouseX, int mouseY, bool mousePressed) {
    if (!active) return;
    
    hoveredButton = -1;
    
    // Get actual renderer size (same as render method)
    if (!gameRef) return;
    SDL_Renderer* renderer = gameRef->getRenderer();
    int actualWidth, actualHeight;
    SDL_GetRendererOutputSize(renderer, &actualWidth, &actualHeight);
    
    // Calculate menu position using actual dimensions
    int menuX = (actualWidth - MENU_WIDTH) / 2;
    int menuY = (actualHeight - MENU_HEIGHT) / 2;
    
    // Calculate button positions and check for hover/clicks
    int buttonStartY = menuY + 120;
    int buttonX = menuX + (MENU_WIDTH - BUTTON_WIDTH) / 2;
    
    for (int i = 0; i < TOTAL_BUTTONS; i++) {
        int buttonY = buttonStartY + i * (BUTTON_HEIGHT + BUTTON_SPACING);
        
        SDL_Rect buttonRect = {buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT};
        
        if (mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
            mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h) {
            
            hoveredButton = i;
            selectedButton = i; // Update keyboard selection to match mouse
            
            // Handle click
            if (mousePressed && !lastMousePressed) {
                MenuButton button = static_cast<MenuButton>(i);
                if (isButtonEnabled(button)) {
                    switch (button) {
                        case MenuButton::CONTINUE: continueGame(); break;
                        case MenuButton::RESTART: restartGame(); break;
                        case MenuButton::EXIT: exitGame(); break;
                    }
                }
            }
        }
    }
    
    lastMousePressed = mousePressed;
}

void Menu::continueGame() {
    if (menuType == MenuType::PAUSE) {
        hideMenu();
    }
}

void Menu::restartGame() {
    if (gameRef) {
        gameRef->restartGame();
    }
}

void Menu::exitGame() {
    if (gameRef) {
        gameRef->exitGame();
    }
}
