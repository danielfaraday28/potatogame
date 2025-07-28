# Brotato MVP Game - Setup Instructions

A Brotato-inspired top-down survival shooter game built with C++ and SDL2.

## 🎮 Game Features

- **Mouse Aiming**: Gun follows your mouse cursor
- **Experience System**: Collect green experience orbs dropped by enemies
- **Character Progression**: Level up and gain stats automatically
- **Wave-Based Survival**: Waves start at 20 seconds and increase by 5 seconds each wave (max 60s)
- **Brotato-Style UI**: Health bar, level display, materials counter, and countdown timer
- **Materials System**: Earn materials by killing enemies
- **Character Stats**: Speed, damage, pickup range, armor, health regen, and more

## 🛠️ Prerequisites

### Required Software
1. **Visual Studio 2019/2022** (with C++ development tools)
2. **Git** for version control
3. **vcpkg** package manager
4. **CMake** (3.16 or higher)

### Windows Environment
- Windows 10/11
- PowerShell (for running commands)

## 📦 Installation Steps

### Step 1: Install vcpkg Package Manager

1. **Clone vcpkg** (if you don't have it):
   ```powershell
   cd C:\
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. **Integrate vcpkg with Visual Studio**:
   ```powershell
   .\vcpkg integrate install
   ```

### Step 2: Install Required Libraries

```powershell
cd C:\vcpkg
.\vcpkg install sdl2:x64-windows
.\vcpkg install sdl2-image:x64-windows
```

### Step 3: Clone/Download the Game

1. **If using Git**:
   ```powershell
   cd C:\Users\[YourUsername]\Documents\GitHub
   git clone [your-repository-url] potatogame
   ```

2. **Or manually**: Copy the game folder to your desired location

### Step 4: Verify Project Structure

Your project should look like this:
```
potatogame/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── Game.cpp
│   ├── Game.h
│   ├── Player.cpp
│   ├── Player.h
│   ├── Enemy.cpp
│   ├── Enemy.h
│   ├── Bullet.cpp
│   ├── Bullet.h
│   ├── Vector2.cpp
│   ├── Vector2.h
│   ├── ExperienceOrb.cpp
│   └── ExperienceOrb.h
└── monsters/
    └── landmonster/
        └── Transparent PNG/
            ├── idle/
            │   ├── frame-1.png
            │   └── frame-2.png
            └── got hit/
                └── frame.png
```

### Step 5: Build the Game

1. **Open PowerShell** in the project directory
2. **Create build directory**:
   ```powershell
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```powershell
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

4. **Build the project**:
   ```powershell
   cmake --build .
   ```

### Step 6: Run the Game

```powershell
.\Debug\BrotatoGame.exe
```

## 🎮 How to Play

### Controls
- **Movement**: WASD or Arrow Keys
- **Aiming**: Move your mouse cursor
- **Shooting**: Spacebar (hold for continuous fire)

### Gameplay
1. **Survive the Waves**: Each wave lasts 20-60 seconds
2. **Kill Enemies**: Shoot the blue monsters that spawn from screen edges
3. **Collect Experience**: Walk over green glowing orbs to gain XP
4. **Level Up**: Automatically gain stats when you have enough XP
5. **Earn Materials**: Get materials for each enemy killed

### UI Elements
- **Top-Left Red Bar**: Your health (X / Y format)
- **Top-Left Gray Box**: Your current level (LV.X)
- **Top-Left Green Circle**: Materials collected
- **Center Top**: Current wave number and countdown timer
- **Bottom Green Bar**: Experience progress to next level

## 🔧 Troubleshooting

### Common Issues

1. **"SDL2 not found" Error**:
   - Make sure vcpkg is properly installed and integrated
   - Verify SDL2 packages are installed: `.\vcpkg list | grep sdl2`

2. **Monster Sprites Not Loading**:
   - Check that the `monsters/` folder is in the same directory as the executable
   - The build process should automatically copy monster assets

3. **Game Won't Start**:
   - Make sure you're running from the `build` directory
   - Check that all dependencies are installed
   - Verify Visual Studio C++ runtime is installed

4. **Build Errors**:
   - Ensure CMake is version 3.16 or higher
   - Make sure vcpkg toolchain path is correct
   - Try cleaning and rebuilding: `cmake --build . --clean-first`

### Performance Issues
- The game runs at ~60 FPS
- If experiencing lag, check Task Manager for other running processes
- Ensure graphics drivers are up to date

## 🚀 Development Notes

### Code Structure
- **Game.cpp/h**: Main game loop, rendering, and state management
- **Player.cpp/h**: Player character, stats, and progression system
- **Enemy.cpp/h**: Enemy AI, sprite animation, and behavior
- **Bullet.cpp/h**: Projectile physics and collision detection
- **ExperienceOrb.cpp/h**: Experience collection system
- **Vector2.cpp/h**: 2D vector math utilities

### Adding Features
- Character stats are defined in `PlayerStats` struct
- Wave timing can be adjusted in `Game.cpp` (waveDuration variable)
- New enemy types can be added by extending the Enemy class
- UI elements are rendered in the `renderUI()` method

### Graphics System
- Uses custom bitmap font rendering (no external font dependencies)
- Sprite animations for enemies
- Particle-like effects for experience orbs
- Simple SDL2 rectangle-based UI elements

## 📋 System Requirements

### Minimum Requirements
- **OS**: Windows 10/11
- **CPU**: Any modern CPU (game is not CPU-intensive)
- **RAM**: 100MB available memory
- **Graphics**: Any GPU with basic 2D acceleration
- **Storage**: ~50MB for game files and dependencies

### Recommended
- **Resolution**: 1024x768 or higher
- **Mouse**: For precise aiming
- **Audio**: Speakers/headphones (if audio is added later)

## 📞 Support

If you encounter issues:
1. Check this README first
2. Verify all prerequisites are installed
3. Make sure vcpkg packages are up to date
4. Try rebuilding the project from scratch
5. Check Windows Event Viewer for detailed error messages

## 🎯 Quick Start Checklist

- [ ] Visual Studio installed with C++ tools
- [ ] vcpkg installed and integrated
- [ ] SDL2 and SDL2_image installed via vcpkg
- [ ] Project files downloaded/cloned
- [ ] Monster sprite assets in correct location
- [ ] Built successfully with CMake
- [ ] Game runs and displays UI correctly

Once all items are checked, you should be able to enjoy the game! 🎮

---

**Version**: 1.0  
**Last Updated**: December 2024  
**Compatible**: Windows 10/11, Visual Studio 2019/2022