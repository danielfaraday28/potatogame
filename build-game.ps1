# Brotato Game Build Script
# This script builds the game and copies it to an easy-to-reach location

Write-Host "🎮 Building Brotato Game..." -ForegroundColor Green
Write-Host ""

# Set error handling
$ErrorActionPreference = "Stop"

try {
    # Check if vcpkg is available
    if (!(Test-Path "C:\vcpkg\scripts\buildsystems\vcpkg.cmake")) {
        Write-Host "❌ vcpkg not found at C:\vcpkg\" -ForegroundColor Red
        Write-Host "Please install vcpkg first or update the path in this script." -ForegroundColor Yellow
        exit 1
    }

    # Check if SDL2 is installed
    Write-Host "📦 Checking SDL2 dependencies..." -ForegroundColor Cyan
    $vcpkgList = & C:\vcpkg\vcpkg.exe list | Out-String
    if ($vcpkgList -notmatch "sdl2:x64-windows") {
        Write-Host "⚠️  Installing SDL2 dependencies..." -ForegroundColor Yellow
        & C:\vcpkg\vcpkg.exe install sdl2:x64-windows sdl2-image:x64-windows
        if ($LASTEXITCODE -ne 0) {
            Write-Host "❌ Failed to install SDL2 dependencies" -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "✅ SDL2 dependencies already installed" -ForegroundColor Green
    }

    # Clean and create build directory
    Write-Host "🧹 Cleaning build directory..." -ForegroundColor Cyan
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
    }
    New-Item -ItemType Directory -Name "build" | Out-Null
    Set-Location "build"

    # Configure with CMake
    Write-Host "⚙️  Configuring with CMake..." -ForegroundColor Cyan
    & cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ CMake configuration failed" -ForegroundColor Red
        Write-Host "Current directory: $(Get-Location)" -ForegroundColor Yellow
        Write-Host "Checking if CMakeLists.txt exists in parent: $(Test-Path '../CMakeLists.txt')" -ForegroundColor Yellow
        Set-Location ".."
        exit 1
    }

    # Verify CMake cache was created
    if (!(Test-Path "CMakeCache.txt")) {
        Write-Host "❌ CMakeCache.txt not found - configuration may have failed silently" -ForegroundColor Red
        Set-Location ".."
        exit 1
    }

    # Build the project
    Write-Host "🔨 Building project..." -ForegroundColor Cyan
    & cmake --build . --config Debug
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Build failed" -ForegroundColor Red
        Set-Location ".."
        exit 1
    }

    # Copy missing assets manually (fix for asset copying issue)
    Write-Host "📁 Fixing asset copying..." -ForegroundColor Cyan
    if (Test-Path "..\assets\ui\card_normal.png") {
        Copy-Item "..\assets\ui\card_normal.png" ".\Debug\assets\ui\" -Force
    }
    if (Test-Path "..\assets\ui\card_selected.png") {
        Copy-Item "..\assets\ui\card_selected.png" ".\Debug\assets\ui\" -Force
    }
    if (Test-Path "..\assets\ui\card_locked.png") {
        Copy-Item "..\assets\ui\card_locked.png" ".\Debug\assets\ui\" -Force
    }
    if (Test-Path "..\assets\ui\coin.png") {
        Copy-Item "..\assets\ui\coin.png" ".\Debug\assets\ui\" -Force
    }
    if (Test-Path "..\assets\ui\lock.png") {
        Copy-Item "..\assets\ui\lock.png" ".\Debug\assets\ui\" -Force
    }
    if (Test-Path "..\assets\ui\lock_locked.png") {
        Copy-Item "..\assets\ui\lock_locked.png" ".\Debug\assets\ui\" -Force
    }
    if (Test-Path "..\assets\ui\reroll.png") {
        Copy-Item "..\assets\ui\reroll.png" ".\Debug\assets\ui\" -Force
    }
    
    # Copy weapon assets if they exist
    if (Test-Path "..\assets\weapons\") {
        Get-ChildItem "..\assets\weapons\*.png" -ErrorAction SilentlyContinue | ForEach-Object {
            Copy-Item $_.FullName ".\Debug\assets\weapons\" -Force
        }
    }

    # Go back to project root
    Set-Location ".."

    # Create easy-access game folder
    $gameFolder = "$env:USERPROFILE\Desktop\BrotatoGame"
    Write-Host "📂 Creating easy-access game folder at: $gameFolder" -ForegroundColor Cyan
    
    if (Test-Path $gameFolder) {
        Remove-Item -Recurse -Force $gameFolder
    }
    New-Item -ItemType Directory -Path $gameFolder | Out-Null

    # Copy game files to desktop folder
    Copy-Item "build\Debug\BrotatoGame.exe" "$gameFolder\"
    Copy-Item "build\Debug\*.dll" "$gameFolder\"
    Copy-Item -Recurse "build\Debug\assets" "$gameFolder\"
    Copy-Item -Recurse "build\Debug\monsters" "$gameFolder\"

    # Create a run script
    $runScript = @"
@echo off
echo 🎮 Starting Brotato Game...
echo.
echo Controls:
echo - Move: WASD
echo - Aim: Mouse
echo - Shoot: Spacebar
echo.
echo Press any key to start the game...
pause > nul
BrotatoGame.exe
"@
    $runScript | Out-File "$gameFolder\Run Game.bat" -Encoding ASCII

    Write-Host ""
    Write-Host "✅ Build completed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "🎯 Game is ready at:" -ForegroundColor Yellow
    Write-Host "   $gameFolder" -ForegroundColor White
    Write-Host ""
    Write-Host "🎮 To play:" -ForegroundColor Yellow
    Write-Host "   1. Go to your Desktop" -ForegroundColor White
    Write-Host "   2. Open the 'BrotatoGame' folder" -ForegroundColor White
    Write-Host "   3. Double-click 'Run Game.bat'" -ForegroundColor White
    Write-Host ""
    Write-Host "🎯 Or run directly:" -ForegroundColor Yellow
    Write-Host "   cd `"$gameFolder`"" -ForegroundColor White
    Write-Host "   .\BrotatoGame.exe" -ForegroundColor White

} catch {
    Write-Host ""
    Write-Host "❌ Build failed with error:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    Write-Host "💡 Try running this script as Administrator if you get permission errors." -ForegroundColor Yellow
    exit 1
}