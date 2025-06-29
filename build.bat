@echo off
setlocal enabledelayedexpansion

REM Delete any previous build directory
if exist build (
    echo [INFO] Removing old 'build' directory...
    rmdir /s /q build
)

mkdir build
cd build

echo [INFO] Configuring with MinGW Makefiles...
cmake .. -G "MinGW Makefiles"

if errorlevel 1 (
    echo [ERROR] CMake configure failed!
    pause
    exit /b 1
)

echo ==================================================
echo [INFO] Building Project...
cmake --build . 

if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo ==================================================
echo [INFO] Build Complete!
pause
endlocal
