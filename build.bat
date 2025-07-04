@echo off
setlocal enabledelayedexpansion

REM === Setup Environment ===
echo [INFO] Setting MinGW64 environment...

:: Use MinGW64 (not UCRT64 or Clang)
set "PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%"
set "PKG_CONFIG_PATH=C:\msys64\mingw64\lib\pkgconfig"

:: Force CMake to use correct compilers
set "CC=C:\msys64\mingw64\bin\gcc.exe"
set "CXX=C:\msys64\mingw64\bin\g++.exe"

REM === Remove previous build ===
if exist build (
    echo [INFO] Removing old 'build' directory...
    rmdir /s /q build
)

mkdir build
cd build

REM === Run CMake configure step ===
echo [INFO] Configuring with MinGW Makefiles...
cmake .. -G "MinGW Makefiles" ^
  -DPKG_CONFIG_EXECUTABLE="C:/msys64/usr/bin/pkg-config.exe"

if errorlevel 1 (
    echo [ERROR] CMake configure failed!
    pause
    exit /b 1
)

REM === Build Project ===
echo ==================================================
echo [INFO] Building Project...
cmake --build .

if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

REM === Done ===
echo ==================================================
echo [INFO] Build Complete!
pause
endlocal
