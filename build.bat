@echo off
setlocal enabledelayedexpansion

echo [INFO] Simple MSYS2 build script

REM === Basic MSYS2 setup ===
set "MSYS_ROOT=C:\msys64"
set "PATH=%MSYS_ROOT%\mingw64\bin;%MSYS_ROOT%\usr\bin;%PATH%"

REM === Clean build ===
if exist build (
    echo [INFO] Removing old build...
    rmdir /s /q build
)
mkdir build
cd build

REM === Simple CMake call ===
echo [INFO] Running CMake...
cmake .. -G "MinGW Makefiles"

if errorlevel 1 (
    echo [ERROR] CMake failed!
    pause
    exit /b 1
)

REM === Build ===
echo [INFO] Building...
cmake --build .

if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

REM === Copy DLLs ===
echo [INFO] Copying DLLs...
copy "%MSYS_ROOT%\mingw64\bin\*.dll" . >nul 2>&1

echo [INFO] Build complete!
echo [INFO] To run: make sure %MSYS_ROOT%\mingw64\bin is in your Windows PATH
pause