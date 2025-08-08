@echo off
setlocal enabledelayedexpansion

echo [INFO] Simple MSYS2 build script

REM === Basic MSYS2 setup ===
if defined MSYS2_ROOT (
    set "MSYS_ROOT=%MSYS2_ROOT%"
) else (
    REM Fall back to common locations
    if exist "C:\msys64" set "MSYS_ROOT=C:\msys64"
    if exist "D:\msys64" set "MSYS_ROOT=D:\msys64"
)

REM === Force using MSYS2 MinGW64 GCC/G++ ===
set "PATH=%MSYS_ROOT%\mingw64\bin;%MSYS_ROOT%\usr\bin;%PATH%"
set "CC=%MSYS_ROOT%\mingw64\bin\gcc.exe"
set "CXX=%MSYS_ROOT%\mingw64\bin\g++.exe"

REM === Clean build ===
if exist build (
    echo [INFO] Removing old build...
    rmdir /s /q build
)
mkdir build
cd build

REM === Simple CMake call ===
echo [INFO] Running CMake...
cmake .. -G "Ninja" ^
    -DOpenCV_DIR="%MSYS_ROOT%/mingw64/lib/cmake/opencv4"

if errorlevel 1 (
    echo [ERROR] CMake failed!
    pause
    exit /b 1
)

REM === Build ===
echo [INFO] Building...
ninja

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
