#pragma once
#include <SDL2/SDL.h>
#include <string>

class Player;
class Game;

enum class MenuType {
    PAUSE,
    GAME_OVER
};

enum class MenuButton {
    CONTINUE = 0,
    RESTART = 1,
    EXIT = 2
};

class Menu {
public:
    Menu();
    ~Menu();
    
    void setGame(Game* game) { gameRef = game; }
    void loadAssets(SDL_Renderer* renderer) { /* No assets to load - using colored rectangles */ }
    void unloadAssets() { /* No assets to unload */ }
    
    void showMenu(MenuType type);
    void hideMenu();
    bool isMenuActive() const { return active; }
    MenuType getMenuType() const { return menuType; }
    
    // State synchronization with Game
    void setEscKeyPressed(bool pressed) { escKeyPressed = pressed; }
    
    void render(SDL_Renderer* renderer, int windowWidth, int windowHeight);
    void handleInput(const Uint8* keyState);
    void handleMouseInput(int mouseX, int mouseY, bool mousePressed);
    
    // Menu actions
    void continueGame();
    void restartGame();
    void exitGame();
    
private:
    bool active;
    MenuType menuType;
    
    // Button navigation
    int selectedButton;
    int hoveredButton;
    bool lastMousePressed;
    
    // Input handling - prevent key repeat
    bool escKeyPressed;
    bool enterKeyPressed;
    bool upKeyPressed;
    bool downKeyPressed;
    bool justShown; // Flag to prevent immediate ESC handling after menu is shown
    
    Game* gameRef;
    

    
    // UI constants
    static const int MENU_WIDTH = 600;
    static const int MENU_HEIGHT = 400;
    static const int BUTTON_WIDTH = 300;
    static const int BUTTON_HEIGHT = 60;
    static const int BUTTON_SPACING = 20;
    static const int TOTAL_BUTTONS = 3;
    
    // Helper methods
    void renderButton(SDL_Renderer* renderer, MenuButton button, int x, int y, bool highlighted, bool disabled);
    void renderBlurBackground(SDL_Renderer* renderer, int windowWidth, int windowHeight);
    void renderTTFText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, int fontSize = 16);
    bool isButtonEnabled(MenuButton button) const;
    std::string getButtonText(MenuButton button) const;
    std::string getMenuTitle() const;
};
