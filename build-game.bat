@echo off
echo 🎮 Brotato Game Builder
echo.
echo This will build your game and put it on your Desktop for easy access.
echo.
pause

powershell -ExecutionPolicy Bypass -File "build-game.ps1"

echo.
echo Press any key to exit...
pause > nul