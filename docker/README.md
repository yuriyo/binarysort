# Docker Build Guide

This directory contains Dockerfiles for building BinarySort on different platforms.

## Available Platforms

1. **Linux (x86_64)** - `Dockerfile.linux`
   - Based on Oracle Linux 9
   - Uses GCC 13 toolset
   - Native x86_64 build

2. **Windows (AMD64)** - `Dockerfile.windows`
   - Cross-compiled on Linux using MinGW-w64
   - Produces native Windows executables

3. **macOS (ARM64 & x86_64)** - `Dockerfile.macos-cross`
   - Cross-compiled using OSXCross
   - **Requires macOS SDK** (not included due to licensing)
   - Produces both ARM64 and Intel binaries

## Building with Docker

### Build All Platforms

```bash
# On Linux/macOS
./dockerbuild.sh all

# On Windows
dockerbuild.bat all
```

### Build Specific Platform

```bash
# Linux only
./dockerbuild.sh linux

# Windows only
./dockerbuild.sh windows

# macOS only (requires SDK setup - see below)
./dockerbuild.sh macos
```

### Output

- `binsort-macos-arm64.tar.gz` (if macOS cross-compile is used)
- `binsort-macos-x64.tar.gz` (if macOS cross-compile is used)

## macOS Cross-Compilation Setup

macOS cross-compilation requires the macOS SDK, which cannot be redistributed due to Apple's licensing.

### Option 1: OSXCross (Docker)

1. Download macOS SDK from [joseluisq/macosx-sdks](https://github.com/joseluisq/macosx-sdks/releases)
2. Create `docker/sdk/` directory
3. Place `MacOSX14.0.sdk.tar.xz` (or similar) in `docker/sdk/`
4. Run: `./dockerbuild.sh macos`

**Note**: You must have legal rights to use the macOS SDK (e.g., Xcode license agreement).

### Option 2: Native Build (Recommended)

For production builds, use native macOS machines or GitHub Actions
## macOS Builds

macOS builds cannot be easily done in Docker due to licensing restrictions. Use the native build:

```bash
# macOS ARM64 (Apple Silicon)
mkdir build && cd build
cmake -DCMAKE_OSX_ARCHITECTURES=arm64 ..
make

# macOS x86_64 (Intel)
mkdir build && cd build
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 ..
make
```

## GitHub Actions

Automated builds are triggered on release tags matching `r*.*.*`:

```bash
git tag r1.0.0
git push origin r1.0.0
```

This will build binaries for all platforms and create a GitHub release.

## Manual Docker Commands

If you prefer to run Docker commands manually:

```bash
# Build Linux
docker build -f docker/Dockerfile.linux -t binsort-builder:linux .
docker run --rm -v $(pwd)/release:/artifacts binsort-builder:linux

# Build Windows
docker build -f docker/Dockerfile.windows -t binsort-builder:windows .
docker run --rm -v $(pwd)/release:/artifacts binsort-builder:windows
```

## Requirements

- Docker installed and running
- Sufficient disk space for build images (~2-3 GB per platform)
- Network access for downloading dependencies

## Troubleshooting

### Docker Permission Errors

On Linux, you may need to run with sudo or add your user to the docker group:
```bash
sudo usermod -aG docker $USER
```

### Build Failures

Check Docker logs:
```bash
docker logs <container-id>
```

Clean and rebuild:
```bash
docker system prune -a
./dockerbuild.sh <platform>
```
