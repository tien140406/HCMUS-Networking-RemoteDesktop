@echo off
setlocal enabledelayedexpansion

echo [INFO] Simple MSYS2 build script

REM === MSYS2 location ===
if defined MSYS2_ROOT (
    set "MSYS_ROOT=%MSYS2_ROOT%"
) else (
    if exist "C:\msys64" set "MSYS_ROOT=C:\msys64"
    if exist "D:\msys64" set "MSYS_ROOT=D:\msys64"
)

REM === Use MinGW64 GCC/G++ ===
set "PATH=%MSYS_ROOT%\mingw64\bin;%MSYS_ROOT%\usr\bin;%PATH%"
set "CC=%MSYS_ROOT%\mingw64\bin\gcc.exe"
set "CXX=%MSYS_ROOT%\mingw64\bin\g++.exe"

REM === Go to project root ===
cd /d %~dp0

REM === Remove old CMake cache from project root ===
if exist CMakeCache.txt (
    echo [INFO] Removing old CMakeCache.txt...
    del /f /q CMakeCache.txt
)
if exist CMakeFiles (
    echo [INFO] Removing old CMakeFiles directory...
    rmdir /s /q CMakeFiles
)

REM === Clean build folder ===
if exist build (
    echo [INFO] Removing old build directory...
    rmdir /s /q build
)
mkdir build
cd build

REM === Run CMake ===
echo [INFO] Running CMake...
cmake .. -G "MinGW Makefiles" ^
    -DOpenCV_DIR="%MSYS_ROOT%/mingw64/lib/cmake/opencv4"

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

REM === Copy DLLs vào build ===
echo [INFO] Copying DLLs...
xcopy "%MSYS_ROOT%\mingw64\bin\*.dll" . /Y >nul

echo [INFO] Build complete!
pause
