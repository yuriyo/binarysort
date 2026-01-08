#!/bin/bash
# Docker-based multi-platform build script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
OUTPUT_DIR="$PROJECT_ROOT/release"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create output directory
mkdir -p "$OUTPUT_DIR"

build_platform() {
    local platform=$1
    local dockerfile=$2
    local image_name="binsort-builder:$platform"
    
    echo_info "Building for $platform..."
    
    # Build Docker image
    docker build -f "$PROJECT_ROOT/docker/$dockerfile" -t "$image_name" "$PROJECT_ROOT"
    
    # Run container and extract artifacts
    docker run --rm -v "$OUTPUT_DIR:/artifacts" "$image_name"
    
    echo_info "✓ $platform build complete"
}

# Parse command line arguments
PLATFORMS=()
if [ $# -eq 0 ]; then
    # Build all platforms by default
    PLATFORMS=("linux" "windows")
    echo_warn "Skipping macOS cross-compilation (requires SDK setup)"
    echo_warn "Use 'dockerbuild.sh macos' to attempt macOS build"
else
    PLATFORMS=("$@")
fi

echo_info "Starting multi-platform builds..."
echo_info "Output directory: $OUTPUT_DIR"
echo ""

for platform in "${PLATFORMS[@]}"; do
    case $platform in
        linux)
            build_platform "linux" "Dockerfile.linux"
            ;;
        windows)
        macos)
            if [ ! -d "$PROJECT_ROOT/docker/sdk" ] || [ -z "$(ls -A $PROJECT_ROOT/docker/sdk/*.tar.xz 2>/dev/null)" ]; then
                echo_error "macOS SDK not found in docker/sdk/"
                echo_info "Download MacOSX SDK from: https://github.com/joseluisq/macosx-sdks"
                echo_info "Place MacOSX*.sdk.tar.xz in docker/sdk/ directory"
                exit 1
            fi
            build_platform "macos" "Dockerfile.macos-cross"
            ;;
        all)
            build_platform "linux" "Dockerfile.linux"
            build_platform "windows" "Dockerfile.windows"
            echo_warn "Skipping macOS (use 'dockerbuild.sh macos' separately)"
            ;;
        *)
            echo_error "Unknown platform: $platform"
            echo_info "Available platforms: linux, windows, maco
            echo_error "Unknown platform: $platform"
            echo_info "Available platforms: linux, windows, all"
            exit 1
            ;;
    esac
    echo ""
done

echo_info "All builds complete!"
echo_info "Artifacts available in: $OUTPUT_DIR"
ls -lh "$OUTPUT_DIR"
