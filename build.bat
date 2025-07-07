@echo off
setlocal enabledelayedexpansion

REM === Setup MSYS2 MinGW64 environment ===
echo [INFO] Setting up MSYS2 MinGW64 environment...
set "PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%"
set "PKG_CONFIG_PATH=C:\msys64\mingw64\lib\pkgconfig"
set "CC=C:\msys64\mingw64\bin\gcc.exe"
set "CXX=C:\msys64\mingw64\bin\g++.exe"

REM === Remove old build ===
if exist build (
    echo [INFO] Removing old 'build' directory...
    rmdir /s /q build
)

mkdir build
cd build

REM === Configure with CMake ===
echo [INFO] Configuring with CMake...
cmake .. -G "MinGW Makefiles" ^
  -DPKG_CONFIG_EXECUTABLE="C:/msys64/usr/bin/pkg-config.exe"

if errorlevel 1 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

REM === Build ===
echo ==================================================
echo [INFO] Building project...
cmake --build .

if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

REM === Copy ALL DLLs from MSYS2 MinGW64 ===
echo ==================================================
echo [INFO] Copying all DLLs from C:\msys64\mingw64\bin ...
for %%f in (C:\msys64\mingw64\bin\*.dll) do (
    if exist "%%f" (
        copy "%%f" . >nul
        echo [OK] Copied %%~nxf
    )
)

REM === Final info ===
echo ==================================================
echo [INFO] Listing copied DLLs:
dir /b *.dll

REM === Auto-set Bash PATH ===
set PATH=%CD%;%PATH%

echo ==================================================
echo [INFO] Build Complete!
pause
endlocal
