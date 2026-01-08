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
```

### Output

All build artifacts are placed in the `release/` directory:
- `binsort-linux-x64.tar.gz`
- `binsort-windows-amd64.zip`

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
