# Binary Sort - Fixed-Record Binary Sort Utility

High-performance C++23 utility for sorting fixed-length binary record files with explicit endianness control.

## Features

- **Multi-platform support**: macOS, Linux, Windows
- **Memory-mapped I/O**: Efficient file access using platform-specific memory mapping
- **Multi-threaded sorting**: Parallel sorting with configurable thread count
- **JIT optimization**: Machine code generation for comparison functions (x64)
- **Explicit endianness**: Support for little-endian and big-endian numeric fields
- **In-place sorting**: Sort files without creating temporary copies

## Building

### Prerequisites

- CMake 3.25 or later
- C++23 compatible compiler:
  - GCC 11+ 
  - Clang 15+
  - MSVC 2022+

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests
ctest

# Install (optional)
cmake --install .
```

## Usage

```bash
binsort <input_file> <output_file> / <parameters>
```

### Parameters

- `sort(pos,len,type,order[,...])` - Sort key specification
  - `pos`: 1-based position in record
  - `len`: Length in bytes (2, 4, or 8 for numeric types)
  - `type`: Key type
    - `c` - Character (byte-wise)
    - `w` - Little-endian integer
    - `W` - Big-endian integer
    - `f` - Little-endian IEEE 754 float
  - `order`: Sort order
    - `a` - Ascending
    - `d` - Descending

- `record(length)` - Record length in bytes

- `thread_count(N)` - Number of threads (default: CPU cores)

### Examples

Sort 16-byte records by multiple keys:
```bash
binsort input.dat output.dat / \
  sort(1,4,w,a,5,4,w,a,9,4,W,a,13,4,w,a) \
  record(16) \
  thread_count(4)
```

In-place sort with descending order:
```bash
binsort data.bin data.bin / \
  sort(1,8,W,d) \
  record(32)
```

## Architecture

### Key Components

1. **Memory Mapper** ([memory_mapper.hpp](include/memory_mapper.hpp))
   - Cross-platform memory mapping abstraction
   - Unix: `mmap()`
   - Windows: `MapViewOfFile()`
   - Supports read-only and read-write modes

2. **Record Layer** ([record.hpp](include/record.hpp))
   - Fixed-length record abstraction
   - Key extraction with endianness handling
   - Multi-key comparison

3. **Comparison Generator** ([comparison_generator.hpp](include/comparison_generator.hpp))
   - JIT compilation of comparison functions
   - x64 machine code generation
   - Fallback to interpreted mode on unsupported platforms

4. **Sort Engine** ([sort_engine.hpp](include/sort_engine.hpp))
   - Parallel quicksort implementation
   - Chunk-based parallelization
   - K-way merge for sorted chunks

5. **File Operations** ([file_operations.hpp](include/file_operations.hpp))
   - File size validation
   - Record alignment checking
   - File copying utilities

## Performance Optimizations

- **x64 SIMD**: Enabled via `-march=native`, `-mavx2`, `-msse4.2`
- **Link-time optimization**: `-flto` (GCC/Clang), `/LTCG` (MSVC)
- **Memory-mapped I/O**: Zero-copy file access
- **Parallel sorting**: Multi-threaded chunk sorting
- **JIT comparison**: Direct machine code execution

## Design Decisions

### Memory Mapping Strategy

1. **In-place sorting**: Maps the target file directly with read-write access
2. **New file creation**: Copies source to destination, then maps for in-place sort
3. **Platform abstraction**: Unified interface across Unix and Windows

### JIT Code Generation

The comparison generator creates optimized x64 assembly for record comparison:
- Eliminates function call overhead
- Avoids DSL interpretation during sorting
- Falls back to interpreted mode on non-x64 platforms

### Thread Safety

- Each thread sorts an independent chunk
- No shared mutable state during parallel phase
- Synchronization only during merge phase

## Building from Source

### Native Build

```bash
mkdir build && cd build
cmake ..
make
```

### Docker Multi-Platform Builds

Build for Linux and Windows using Docker:

```bash
# Build all platforms
./dockerbuild.sh all

# Build specific platform
./dockerbuild.sh linux
./dockerbuild.sh windows
```

Build artifacts are placed in the `release/` directory.

For macOS builds, use native CMake (Docker not supported):
```bash
# ARM64 (Apple Silicon)
cmake -DCMAKE_OSX_ARCHITECTURES=arm64 ..

# x86_64 (Intel)
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 ..
```

See [docker/README.md](docker/README.md) for detailed build instructions.

## Releases

Automated builds are triggered by creating release tags:

```bash
git tag r1.0.0
git push origin r1.0.0
```

This creates a GitHub release with binaries for:
- Linux x86_64 (Oracle Linux 9)
- Windows AMD64 (MinGW-w64)
- macOS ARM64 (Apple Silicon)
- macOS x86_64 (Intel)

## Testing

Run the test suite:
```bash
cd build
ctest --verbose
```

Test coverage includes:
- Endianness conversion
- Key extraction
- Comparison functions
- Memory mapping
- Sorting correctness

## Limitations & Future Work

- **Current limitations**:
  - JIT code generation is x64-only
  - No stable sort option
  - Limited to in-memory sorting

- **Planned enhancements**:
  - ARM64 JIT support
  - External merge sort for large files
  - Packed decimal support
  - Locale-aware character sorting

## License

MIT License - see [LICENSE](LICENSE) file for details.

## References

- Design Document: [design_document_fixed_record_binary_sort_utility.md](design_document_fixed_record_binary_sort_utility.md)
- C++23 Standard Library
- System V ABI (x64 calling convention)
