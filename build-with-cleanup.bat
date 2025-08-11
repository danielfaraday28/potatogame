@echo off
echo 🎮 Brotato Build with Process Cleanup
echo ====================================
echo.

REM Kill any running game processes
echo 🔄 Stopping any running BrotatoGame processes...
taskkill /f /im "BrotatoGame.exe" 2>nul
if %errorlevel% equ 0 (
    echo ✅ Stopped running BrotatoGame processes
    timeout /t 2 /nobreak >nul
) else (
    echo ℹ️  No BrotatoGame processes were running
)

REM Clean build directory more aggressively
echo 🧹 Cleaning build directory...
if exist "build" (
    echo Removing build directory...
    rmdir /s /q "build" 2>nul
    if exist "build" (
        echo ⚠️  Some files locked, trying harder...
        timeout /t 1 /nobreak >nul
        rmdir /s /q "build" 2>nul
    )
)

REM Wait a moment for file system
timeout /t 1 /nobreak >nul

mkdir "build"
cd "build"

echo.
echo ⚙️ Configuring with CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
if %errorlevel% neq 0 (
    echo ❌ CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo.
echo 🔨 Building project...
cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo ❌ Build failed at linking stage
    echo.
    echo 💡 This usually means the .exe file is locked. Let me try to fix it...
    
    REM Try to unlock and rebuild
    echo 🔄 Killing any processes that might be locking files...
    taskkill /f /im "BrotatoGame.exe" 2>nul
    taskkill /f /im "msbuild.exe" 2>nul
    taskkill /f /im "cl.exe" 2>nul
    taskkill /f /im "link.exe" 2>nul
    
    timeout /t 2 /nobreak >nul
    
    echo 🔨 Trying build again...
    cmake --build . --config Debug
    
    if %errorlevel% neq 0 (
        echo ❌ Build still failed
        echo.
        echo 🔧 Manual steps to try:
        echo 1. Close any running BrotatoGame.exe
        echo 2. Close Visual Studio if open
        echo 3. Wait 30 seconds
        echo 4. Run this script again
        echo.
        cd ..
        pause
        exit /b 1
    )
)

echo.
echo ✅ Build successful!
echo 📁 Copying missing assets...

REM Copy any missing UI assets
if exist "..\assets\ui\card_normal.png" copy "..\assets\ui\card_normal.png" ".\Debug\assets\ui\" >nul 2>&1
if exist "..\assets\ui\card_selected.png" copy "..\assets\ui\card_selected.png" ".\Debug\assets\ui\" >nul 2>&1
if exist "..\assets\ui\card_locked.png" copy "..\assets\ui\card_locked.png" ".\Debug\assets\ui\" >nul 2>&1
if exist "..\assets\ui\coin.png" copy "..\assets\ui\coin.png" ".\Debug\assets\ui\" >nul 2>&1
if exist "..\assets\ui\lock.png" copy "..\assets\ui\lock.png" ".\Debug\assets\ui\" >nul 2>&1
if exist "..\assets\ui\reroll.png" copy "..\assets\ui\reroll.png" ".\Debug\assets\ui\" >nul 2>&1

cd ..

echo.
echo 🎯 Creating Desktop game folder...
set GAME_FOLDER=%USERPROFILE%\Desktop\BrotatoGame

REM Clean desktop folder
if exist "%GAME_FOLDER%" (
    rmdir /s /q "%GAME_FOLDER%" 2>nul
    timeout /t 1 /nobreak >nul
)

mkdir "%GAME_FOLDER%"

echo 📋 Copying game files to Desktop...
copy "build\Debug\BrotatoGame.exe" "%GAME_FOLDER%\" >nul
copy "build\Debug\*.dll" "%GAME_FOLDER%\" >nul
xcopy "build\Debug\assets" "%GAME_FOLDER%\assets" /s /e /i /q >nul 2>&1
xcopy "build\Debug\monsters" "%GAME_FOLDER%\monsters" /s /e /i /q >nul 2>&1

REM Create launcher
echo @echo off > "%GAME_FOLDER%\Run Game.bat"
echo echo 🎮 Starting Brotato Game... >> "%GAME_FOLDER%\Run Game.bat"
echo echo. >> "%GAME_FOLDER%\Run Game.bat"
echo echo Controls: Move=WASD, Aim=Mouse, Shoot=Spacebar >> "%GAME_FOLDER%\Run Game.bat"
echo echo. >> "%GAME_FOLDER%\Run Game.bat"
echo pause >> "%GAME_FOLDER%\Run Game.bat"
echo BrotatoGame.exe >> "%GAME_FOLDER%\Run Game.bat"

echo.
echo ✅ SUCCESS! Game built and ready!
echo.
echo 🎮 Your game is at: %GAME_FOLDER%
echo.
echo 🚀 To play:
echo    1. Go to Desktop
echo    2. Open 'BrotatoGame' folder
echo    3. Double-click 'Run Game.bat'
echo.
echo 🎯 Or run directly: cd "%GAME_FOLDER%" && BrotatoGame.exe
echo.
pause