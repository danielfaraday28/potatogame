@echo off
echo 🔍 Brotato Game Build Diagnostics
echo ================================
echo.

echo Current directory: %CD%
echo.

echo Checking for required files:
echo - CMakeLists.txt: 
if exist "CMakeLists.txt" (echo   ✅ Found) else (echo   ❌ Missing)
echo - src folder: 
if exist "src" (echo   ✅ Found) else (echo   ❌ Missing)
echo - vcpkg: 
if exist "C:\vcpkg\vcpkg.exe" (echo   ✅ Found at C:\vcpkg\) else (echo   ❌ Missing at C:\vcpkg\)
echo.

echo Checking vcpkg integration:
if exist "C:\vcpkg\vcpkg.exe" (
    echo Running: C:\vcpkg\vcpkg.exe list
    C:\vcpkg\vcpkg.exe list
) else (
    echo vcpkg not found - skipping package check
)
echo.

echo Checking CMake:
cmake --version
echo.

echo Checking Visual Studio tools:
where cl.exe
echo.

echo ================================
echo Press any key to continue with manual build attempt...
pause > nul
echo.

echo Attempting manual build:
echo.

echo Step 1: Clean build directory
if exist "build" (
    echo Removing existing build directory...
    rmdir /s /q "build"
)
mkdir build
echo Created build directory

echo.
echo Step 2: Enter build directory
cd build
echo Current directory: %CD%

echo.
echo Step 3: Configure with CMake
echo Running: cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
echo CMake exit code: %errorlevel%

if %errorlevel% neq 0 (
    echo.
    echo ❌ CMake configuration failed!
    echo Checking what files were created:
    dir
    echo.
    cd ..
    echo Press any key to exit...
    pause > nul
    exit /b 1
)

echo.
echo Step 4: Check if configuration succeeded
if exist "CMakeCache.txt" (
    echo ✅ CMakeCache.txt found - configuration successful
) else (
    echo ❌ CMakeCache.txt missing - configuration failed
    cd ..
    echo Press any key to exit...
    pause > nul
    exit /b 1
)

echo.
echo Step 5: Build the project
echo Running: cmake --build . --config Debug
cmake --build . --config Debug
echo Build exit code: %errorlevel%

if %errorlevel% neq 0 (
    echo.
    echo ❌ Build failed!
    cd ..
    echo Press any key to exit...
    pause > nul
    exit /b 1
)

echo.
echo Step 6: Check build results
if exist "Debug\BrotatoGame.exe" (
    echo ✅ BrotatoGame.exe created successfully!
    dir Debug
) else (
    echo ❌ BrotatoGame.exe not found
    echo Build directory contents:
    dir Debug
)

cd ..
echo.
echo ================================
echo Diagnostics complete!
echo Press any key to exit...
pause > nul