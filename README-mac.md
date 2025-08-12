# Brotato MVP Game - macOS Build Guide

A Brotato-inspired top-down survival shooter built with C++ and SDL2.

This guide covers building and running the game on macOS (Apple Silicon and Intel).

## 🛠️ Prerequisites

- Xcode Command Line Tools
- Homebrew (recommended)
- Git
- CMake (3.16+)
- vcpkg (for SDL2 dependencies)

### Install tools
```bash
# Install Homebrew if you don't have it
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Core tools
brew install cmake git ninja
```

### Install vcpkg
```bash
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"

# Bootstrap vcpkg
"$HOME/vcpkg/bootstrap-vcpkg.sh"
```

### Install libraries via vcpkg
Detect your CPU architecture and install the appropriate triplet:

- Apple Silicon (arm64): `arm64-osx`
- Intel (x86_64): `x64-osx`

```bash
# Detect arch
arch=$(uname -m)
triplet=$([ "$arch" = "arm64" ] && echo arm64-osx || echo x64-osx)

"$HOME/vcpkg/vcpkg" install sdl2 sdl2-image sdl2-ttf --triplet "$triplet"
```

## 📦 Project Structure
```
potatogame/
├── CMakeLists.txt
├── README-mac.md
├── src/
│   ├── main.cpp
│   ├── Game.cpp/.h
│   ├── Player.cpp/.h
│   ├── Enemy*.cpp/.h
│   ├── Bullet.cpp/.h
│   ├── Vector2.cpp/.h
│   ├── ExperienceOrb.cpp/.h
│   ├── Material.cpp/.h
│   ├── Weapon.cpp/.h
│   └── Shop.cpp/.h
├── assets/
└── monsters/
```

## 🚧 Build (macOS)

You can use the provided script (recommended) or run commands manually.

### Option A: Use the script (recommended)
```bash
# From the repo root
chmod +x ./build-game-mac.sh
./build-game-mac.sh --config Debug   # or --config Release
```
- The script will:
  - Detect Apple Silicon vs Intel and set the correct vcpkg triplet
  - Configure CMake with the vcpkg toolchain
  - Build the project using Ninja (if available) or Makefiles
  - Copy game assets next to the executable

Run the game after a successful build:
```bash
./build/macos-<triplet>-<config>/BrotatoGame
```
Examples:
- Apple Silicon Debug: `./build/macos-arm64-osx-Debug/BrotatoGame`
- Intel Release: `./build/macos-x64-osx-Release/BrotatoGame`

### Option B: Manual commands
```bash
# From the repo root
VCPKG_ROOT="$HOME/vcpkg"
arch=$(uname -m)
triplet=$([ "$arch" = "arm64" ] && echo arm64-osx || echo x64-osx)

# Create a build directory
mkdir -p build/macos-$triplet-Debug
cd build/macos-$triplet-Debug

# Configure with CMake (+ vcpkg toolchain)
cmake ../.. \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET="$triplet"

# Build
cmake --build . --config Debug

# Run
./BrotatoGame
```

### Option C: CMake Presets (added)
Presets are provided in `CMakePresets.json`.
```bash
# Apple Silicon
export VCPKG_ROOT="$HOME/vcpkg"
cmake --preset macos-arm64-Debug
cmake --build --preset build-macos-arm64-Debug

# Intel
export VCPKG_ROOT="$HOME/vcpkg"
cmake --preset macos-x64-Release
cmake --build --preset build-macos-x64-Release
```

## ▶️ Controls and UI
- Movement: WASD or Arrow Keys
- Aiming: Mouse
- Shooting: Spacebar (hold to fire)
- UI: Health, Level, Materials, Wave timer, XP bar

## 🔧 Troubleshooting (macOS)
- SDL2 not found during configure:
  - Ensure `VCPKG_ROOT` is correct and vcpkg is bootstrapped
  - Ensure you installed `sdl2`, `sdl2-image`, `sdl2-ttf` for the correct triplet
  - Pass `-DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"`
- Linker errors referencing SDL on Apple Silicon:
  - Verify your triplet is `arm64-osx` and not `x64-osx`
- Xcode generator specifics:
  - If you prefer `-G Xcode`, use `cmake --build . --config Debug` and the app will be under the corresponding config directory

## 🧩 Notes
- Asset folders (`assets/`, `monsters/`) are copied next to the built executable automatically.
- The project uses C++17 and SDL2 via CMake `find_package(... CONFIG REQUIRED)`, which works seamlessly with vcpkg.

---

**Tested on**: macOS 14 (Sonoma), Apple Silicon. Should also work on Intel Macs.
