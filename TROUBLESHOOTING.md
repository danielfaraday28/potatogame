# Troubleshooting Guide

## Build Issues

### "Could not find SDL2" or "Could not find SDL2_image"
```
CMake Error: Could not find a package configuration file provided by "SDL2"
```

**Solution:**
1. Ensure vcpkg is installed and integrated:
   ```powershell
   cd C:\vcpkg
   .\vcpkg integrate install
   ```

2. Install SDL2 packages:
   ```powershell
   .\vcpkg install sdl2:x64-windows sdl2-image:x64-windows
   ```

3. Use correct CMake command:
   ```powershell
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

### "MSBuild not found" or Compiler Errors
**Solution:**
- Install Visual Studio 2019/2022 with "Desktop development with C++" workload
- Ensure Windows SDK is installed
- Try running from "Developer Command Prompt for VS"

### CMake Configure Errors
**Solution:**
1. Delete build folder and start fresh:
   ```powershell
   rmdir /s build
   mkdir build
   cd build
   ```

2. Verify CMake version:
   ```powershell
   cmake --version  # Should be 3.16+
   ```

## Runtime Issues

### Game Window Opens But Crashes Immediately
**Check console output for errors:**

1. **Missing DLLs:**
   ```
   Error: SDL2.dll not found
   ```
   - Ensure vcpkg integrated properly
   - Try copying DLLs from vcpkg to build directory

2. **Asset Loading Errors:**
   ```
   Unable to load image monsters/landmonster/...
   ```
   - Verify monster sprites are in correct location
   - Check CMakeLists.txt copies assets properly

### No Audio (Expected Behavior)
- Game currently has no audio system
- This is normal and doesn't affect gameplay

### Performance Issues

**Low FPS or Stuttering:**
1. Check CPU usage in Task Manager
2. Close other applications
3. Update graphics drivers
4. Try running in Release mode:
   ```powershell
   cmake --build . --config Release
   .\Release\BrotatoGame.exe
   ```

**Memory Issues:**
- Game should use <100MB RAM
- If higher, check for memory leaks in development

## Gameplay Issues

### Mouse Aiming Not Working
- Ensure game window has focus (click on it)
- Try moving mouse around the window
- Check if gun barrel (red line) follows mouse

### Can't Pick Up Experience Orbs
- Walk directly over green glowing orbs
- Green circle around player shows pickup range
- If still not working, check console for errors

### Enemies Not Spawning
- Wait for wave to start (check countdown timer)
- Enemies spawn from screen edges
- If none appear after 10+ seconds, restart game

### UI Elements Missing/Incorrect
- Health bar should be red in top-left
- Level should show "LV.X" below health
- Timer should be large numbers in center-top
- If missing, check for font rendering errors in console

## Development Issues

### Code Changes Not Taking Effect
1. Rebuild project:
   ```powershell
   cmake --build . --clean-first
   ```

2. If still issues, delete build folder and reconfigure:
   ```powershell
   rmdir /s build
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build .
   ```

### Debugging the Game
1. Build in Debug mode (default)
2. Use Visual Studio debugger:
   - Open .sln file in build directory
   - Set BrotatoGame as startup project
   - Press F5 to debug

### Adding New Features
- Modify source files in `src/` directory
- Rebuild with `cmake --build .`
- New files need to be added to CMakeLists.txt

## Environment Issues

### PowerShell Execution Policy
```
execution of scripts is disabled on this system
```

**Solution:**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Path Issues
- Ensure vcpkg path is correct in CMake command
- Use forward slashes or double backslashes in paths
- Check that project path has no spaces or special characters

### Antivirus Interference
- Some antivirus software may block the executable
- Add build directory to antivirus exclusions
- Temporarily disable real-time protection during development

## Getting Help

### Diagnostic Information to Collect
1. **System Info:**
   - Windows version
   - Visual Studio version
   - CMake version (`cmake --version`)
   - vcpkg version

2. **Error Messages:**
   - Full CMake configure output
   - Build error messages
   - Console output when running game

3. **File Structure:**
   - Verify project layout matches README
   - Check if assets exist in correct locations

### Common Solutions Summary
1. **90% of issues**: vcpkg not properly integrated or packages not installed
2. **5% of issues**: Missing Visual Studio C++ tools
3. **3% of issues**: Asset files missing or in wrong location
4. **2% of issues**: Other (Windows updates, antivirus, etc.)

### When All Else Fails
1. Start completely fresh:
   - Reinstall vcpkg
   - Reinstall SDL2 packages
   - Download/clone project again
   - Follow README step-by-step

2. Try on different machine to isolate environment issues

3. Check Windows Event Viewer for system-level errors 