@echo off
echo 🎮 Quick Brotato Game Builder
echo.

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo ❌ Error: CMakeLists.txt not found!
    echo Make sure you're running this from the project root directory.
    echo Current directory: %CD%
    pause
    exit /b 1
)

REM Check if vcpkg exists
if not exist "C:\vcpkg\vcpkg.exe" (
    echo ❌ Error: vcpkg not found at C:\vcpkg\
    echo Please install vcpkg first.
    pause
    exit /b 1
)

echo 📦 Installing SDL2 dependencies (if needed)...
C:\vcpkg\vcpkg.exe install sdl2:x64-windows sdl2-image:x64-windows
if errorlevel 1 (
    echo ❌ Failed to install dependencies
    pause
    exit /b 1
)

echo 🧹 Cleaning build directory...
if exist "build" rmdir /s /q "build"
mkdir build
cd build

echo ⚙️ Configuring with CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
if errorlevel 1 (
    echo ❌ CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo 🔨 Building project...
cmake --build . --config Debug
if errorlevel 1 (
    echo ❌ Build failed
    cd ..
    pause
    exit /b 1
)

echo 📁 Copying missing assets...
if exist "..\assets\ui\card_normal.png" copy "..\assets\ui\card_normal.png" ".\Debug\assets\ui\" >nul
if exist "..\assets\ui\card_selected.png" copy "..\assets\ui\card_selected.png" ".\Debug\assets\ui\" >nul
if exist "..\assets\ui\card_locked.png" copy "..\assets\ui\card_locked.png" ".\Debug\assets\ui\" >nul
if exist "..\assets\ui\coin.png" copy "..\assets\ui\coin.png" ".\Debug\assets\ui\" >nul
if exist "..\assets\ui\lock.png" copy "..\assets\ui\lock.png" ".\Debug\assets\ui\" >nul
if exist "..\assets\ui\reroll.png" copy "..\assets\ui\reroll.png" ".\Debug\assets\ui\" >nul

cd ..

echo 📂 Creating Desktop game folder...
set GAME_FOLDER=%USERPROFILE%\Desktop\BrotatoGame
if exist "%GAME_FOLDER%" rmdir /s /q "%GAME_FOLDER%"
mkdir "%GAME_FOLDER%"

echo 📋 Copying game files...
copy "build\Debug\BrotatoGame.exe" "%GAME_FOLDER%\" >nul
copy "build\Debug\*.dll" "%GAME_FOLDER%\" >nul
xcopy "build\Debug\assets" "%GAME_FOLDER%\assets" /s /e /i /q >nul
xcopy "build\Debug\monsters" "%GAME_FOLDER%\monsters" /s /e /i /q >nul

echo @echo off > "%GAME_FOLDER%\Run Game.bat"
echo echo 🎮 Starting Brotato Game... >> "%GAME_FOLDER%\Run Game.bat"
echo echo. >> "%GAME_FOLDER%\Run Game.bat"
echo echo Controls: >> "%GAME_FOLDER%\Run Game.bat"
echo echo - Move: WASD >> "%GAME_FOLDER%\Run Game.bat"
echo echo - Aim: Mouse >> "%GAME_FOLDER%\Run Game.bat"
echo echo - Shoot: Spacebar >> "%GAME_FOLDER%\Run Game.bat"
echo echo. >> "%GAME_FOLDER%\Run Game.bat"
echo BrotatoGame.exe >> "%GAME_FOLDER%\Run Game.bat"

echo.
echo ✅ Build completed successfully!
echo.
echo 🎯 Game is ready at: %GAME_FOLDER%
echo.
echo 🎮 To play:
echo    1. Go to your Desktop
echo    2. Open the 'BrotatoGame' folder  
echo    3. Double-click 'Run Game.bat'
echo.
pause