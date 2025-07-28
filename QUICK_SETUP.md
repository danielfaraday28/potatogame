# Quick Setup Guide

For developers familiar with C++/CMake/vcpkg:

## Prerequisites
- Visual Studio 2019/2022 with C++ tools
- vcpkg package manager
- CMake 3.16+

## Quick Commands

```powershell
# Install dependencies
cd C:\vcpkg
.\vcpkg install sdl2:x64-windows sdl2-image:x64-windows

# Build game
cd [project-directory]
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .

# Run
.\Debug\BrotatoGame.exe
```

## Controls
- **Move**: WASD
- **Aim**: Mouse
- **Shoot**: Spacebar

## Key Features
- Mouse aiming system
- Wave-based survival (20s → 60s waves)
- Experience collection and leveling
- Brotato-style UI with bitmap fonts
- Material collection system

That's it! 🎮 