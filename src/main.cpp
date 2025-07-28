#include "Game.h"
#include <iostream>

int main(int argc, char* args[]) {
    Game game;
    
    if (!game.init()) {
        std::cout << "Failed to initialize game!" << std::endl;
        return -1;
    }
    
    game.run();
    
    return 0;
}