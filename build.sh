#!/bin/bash
# Quick build and test script

set -e

echo "Building Binary Sort..."
mkdir -p build
cd build

cmake ..
cmake --build .

echo ""
echo "Running tests..."
ctest --verbose

echo ""
echo "Build completed successfully!"
echo "Executable: build/binsort"
