# Release Notes

## v1.0.0 (Upcoming)

Initial release of BinarySort - high-performance multi-threaded binary sort utility.

### Features
- Multi-threaded parallel sorting with configurable thread count
- Memory-mapped I/O for zero-copy performance
- Support for in-place and new-file sorting modes
- Multiple key types: byte, word (2-byte), dword (4-byte), qword (8-byte)
- Explicit endianness control (little-endian and big-endian)
- Ascending and descending sort orders
- Multi-key sorting with up to 256 keys

### Performance
- Tested with 50 million records (763 MB dataset)
- Throughput: 30+ MB/s with 4 threads
- Processing rate: 2+ million records/second
- Near-linear thread scaling (3.81× speedup at 4 threads)
- Memory-efficient: uses mmap for zero-copy I/O

### Platform Support
- Linux x86_64 (Oracle Linux 9 with GCC 13)
- Windows AMD64 (MinGW-w64 cross-compiled)
- macOS ARM64 (Apple Silicon M1/M2/M3)
- macOS x86_64 (Intel)

### Build System
- CMake 3.25+ with C++23 support
- Docker-based cross-platform builds
- GitHub Actions automated release pipeline

### Documentation
- Complete user guide with examples
- Test reports and validation results
- Large-scale performance benchmarks
- Docker build guide

### Known Limitations
- JIT comparison generator uses interpreted fallback (framework in place for future x64 code generation)
- Thread scaling plateaus beyond 8 threads on tested hardware
- Requires C++23 capable compiler

### License
MIT License - see LICENSE file for details
