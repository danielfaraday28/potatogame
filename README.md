# Brotato MVP Game

A simple Brotato-inspired top-down survival shooter built in C++ using SDL2.

## Features

- **Player Character**: Orange potato that moves with WASD keys
- **Shooting System**: Shoot bullets with SPACE, aim with IJKL keys
- **Enemy AI**: Red enemies spawn from screen edges and chase the player
- **Collision Detection**: Bullets destroy enemies, enemies damage player
- **Wave System**: Enemy spawn rate increases over time
- **Health System**: Player has 100 HP, loses health when hit by enemies

## Controls

- **Movement**: WASD or Arrow Keys
- **Aim**: I (up), J (left), K (down), L (right)  
- **Shoot**: SPACE
- **Quit**: Close window or Alt+F4

## Build Requirements

- CMake 3.16+
- SDL2
- SDL2_image
- C++17 compatible compiler

## Building on Windows

1. Install SDL2 development libraries
2. Create build directory: `mkdir build && cd build`
3. Generate project: `cmake ..`
4. Build: `cmake --build .`
5. Run: `./BrotatoGame.exe`

## Building on Linux/Mac

```bash
# Install SDL2 (Ubuntu/Debian)
sudo apt-get install libsdl2-dev libsdl2-image-dev

# Build
mkdir build && cd build
cmake ..
make
./BrotatoGame
```

## Gameplay

Survive as long as possible! Enemies spawn continuously and become more frequent over time. Use your shooting skills to destroy enemies before they reach you. The game ends when your health reaches zero.

Score increases by 10 points for each enemy destroyed.