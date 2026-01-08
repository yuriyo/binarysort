@echo off
REM Docker-based multi-platform build script for Windows

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%
set OUTPUT_DIR=%PROJECT_ROOT%release

echo [INFO] Starting multi-platform builds...
echo [INFO] Output directory: %OUTPUT_DIR%
echo.

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM Parse command line arguments
set PLATFORM=%1
if "%PLATFORM%"=="" set PLATFORM=all

if "%PLATFORM%"=="linux" (
    call :build_linux
    goto :done
)

if "%PLATFORM%"=="windows" (
    call :build_windows
    goto :done
)

if "%PLATFORM%"=="all" (
    call :build_linux
    call :build_windows
    echo [WARN] Skipping macOS (use 'dockerbuild.bat macos' separately)
    goto :done
)

if "%PLATFORM%"=="macos" (
    call :build_macos
    goto :done
)

echo [ERROR] Unknown platform: %PLATFORM%
echo [INFO] Available platforms: linux, windows, macos, all
exit /b 1

:build_linux
echo [INFO] Building for Linux...
docker build -f "%PROJECT_ROOT%docker\Dockerfile.linux" -t binsort-builder:linux "%PROJECT_ROOT%"
docker run --rm -v "%OUTPUT_DIR%:/artifacts" binsort-builder:linux
echo [INFO] Linux build complete
echo.
exit /b 0

:build_windows
echo [INFO] Building for Windows...
docker build -f "%PROJECT_ROOT%docker\Dockerfile.windows" -t binsort-builder:windows "%PROJECT_ROOT%"
docker run --rm -v "%OUTPUT_DIR%:/artifacts" binsort-builder:windows
echo [INFO] Windows build complete
echo.
exit /b 0

:build_macos
if not exist "%PROJECT_ROOT%docker\sdk" (
    echo [ERROR] macOS SDK not found in docker\sdk\
    echo [INFO] Download MacOSX SDK from: https://github.com/joseluisq/macosx-sdks
    echo [INFO] Place MacOSX*.sdk.tar.xz in docker\sdk\ directory
    exit /b 1
)
echo [INFO] Building for macOS (cross-compile)...
docker build -f "%PROJECT_ROOT%docker\Dockerfile.macos-cross" -t binsort-builder:macos "%PROJECT_ROOT%"
docker run --rm -v "%OUTPUT_DIR%:/artifacts" binsort-builder:macos
echo [INFO] macOS build complete
echo.
exit /b 0

:done
echo [INFO] All builds complete!
echo [INFO] Artifacts available in: %OUTPUT_DIR%
dir "%OUTPUT_DIR%"
exit /b 0
