@echo off
echo Simple Brotato Build
echo ===================
echo.

REM Go to project directory (in case we're not there)
cd /d "%~dp0"

REM Clean build
if exist build rmdir /s /q build
mkdir build
cd build

REM Configure
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
if errorlevel 1 goto error

REM Build  
cmake --build . --config Debug
if errorlevel 1 goto error

REM Success
echo.
echo ✅ Build successful!
echo Game is at: %CD%\Debug\BrotatoGame.exe
echo.
echo To run: cd build\Debug && BrotatoGame.exe
echo.
pause
exit /b 0

:error
echo.
echo ❌ Build failed!
echo.
pause
exit /b 1