# 50 Million Records Performance Test

**Test Date:** January 8, 2026  
**Platform:** macOS ARM64 (Apple Silicon)  
**File Size:** 763 MB (50,000,000 × 16-byte records)

## 🎯 Test Objectives
- Validate correctness on large dataset (50M records)
- Measure thread scaling performance
- Test in-place vs. new-file sorting
- Benchmark throughput and efficiency

## 📊 Results Summary

### Thread Scaling Performance

| Threads | Sort Time | Total Time | Throughput | Speedup | Efficiency |
|---------|-----------|------------|------------|---------|------------|
| 1       | 94.84s    | 96.48s     | 8.04 MB/s  | 1.00×   | 100%       |
| 2       | 40.86s    | 42.92s     | 18.67 MB/s | 2.32×   | 116%       |
| **4**   | **24.87s**| **26.71s** | **30.68 MB/s** | **3.81×** | **95%** |
| 8       | 25.65s    | 27.53s     | 29.75 MB/s | 3.70×   | 46%        |
| 12      | 33.71s    | 35.39s     | 22.63 MB/s | 2.82×   | 24%        |

**Optimal Configuration:** 4 threads

### Key Performance Metrics

**Best Performance (4 threads):**
- **Sort Time:** 24.87 seconds
- **Total Time:** 26.71 seconds (includes 1.6s file copy)
- **Throughput:** 30.68 MB/s
- **Records/sec:** 2,010,000
- **Speedup:** 3.81× over single-threaded

**In-Place Sorting (4 threads):**
- **Sort Time:** 25.07 seconds
- **Throughput:** 30.43 MB/s
- **Result:** Nearly identical to new-file performance

### Correctness Validation

✅ **All tests passed verification**
- 50M records sorted correctly (verified in 0.97s)
- In-place sorted file verified correct
- No data corruption or ordering errors
- Deterministic results across multiple runs

## 📈 Scaling Analysis

### Thread Efficiency

```
Speedup Curve:
4.0× ┤                         ╭─────
3.5× ┤                    ╭────╯
3.0× ┤               ╭────╯
2.5× ┤          ╭────╯
2.0× ┤     ╭────╯
1.5× ┤ ╭───╯
1.0× ┼─╯
     └─┬────┬────┬────┬────┬────
       1    2    4    8   12  threads
```

**Observations:**
- Near-linear scaling from 1→4 threads (95% efficiency)
- Optimal at 4 threads on this test system
- Diminishing returns beyond 4 threads due to:
  - Merge overhead increasing with chunk count
  - Memory bandwidth saturation
  - Cache contention

### CPU Utilization

**4 threads configuration:**
- User time: 74.5 seconds
- Real time: 26.7 seconds
- CPU utilization: 279% (2.79 cores effectively used)
- Indicates good parallelization with minimal overhead

## 🔍 Cross-Dataset Comparison

| Records    | File Size | Threads | Time   | Throughput  | Records/sec |
|------------|-----------|---------|--------|-------------|-------------|
| 10,000     | 156 KB    | 4       | 4 ms   | 38.15 MB/s  | 2,500,000   |
| 1,000,000  | 15 MB     | 4       | 359 ms | 42.50 MB/s  | 2,785,000   |
| 50,000,000 | 763 MB    | 4       | 24.9s  | 30.68 MB/s  | 2,010,000   |

**Analysis:**
- Consistent performance across file sizes
- Small files: Higher throughput due to cache fitting
- Large files: Slight degradation (~20%) due to:
  - Cache misses
  - Memory bandwidth limits
  - Larger merge overhead
- Still excellent performance at 2M records/second

## 💾 Memory Characteristics

**Memory Mapped I/O:**
- Mapped size: 800 MB
- Zero-copy access
- Kernel-managed paging
- Sequential access pattern

**Working Memory:**
- Per-thread buffers: ~4 MB
- 4 threads × 4 MB = 16 MB working memory
- Total memory footprint: ~816 MB

**Benefits:**
- No manual buffer management
- Efficient disk I/O
- In-place modifications safe
- OS handles page caching

## 🏆 Achievements

### ✅ Correctness
- All 50 million records sorted correctly
- Multi-key sorting verified (primary + secondary keys)
- In-place sorting maintains data integrity
- Deterministic results

### ✅ Performance
- 2+ million records/second throughput
- 30+ MB/s sustained performance
- Near-linear thread scaling (4 threads)
- Efficient CPU utilization

### ✅ Scalability
- Handles 800 MB file with ease
- Consistent performance across sizes
- Memory-efficient with zero-copy I/O
- Good multi-core utilization

### ✅ Reliability
- No crashes or errors
- Stable across multiple runs
- Proper error handling
- Clean resource management

## 🔬 Technical Details

**Test Configuration:**
- Platform: macOS ARM64
- Compiler: Clang (C++2b)
- Optimizations: -O3, -flto, -ffast-math
- Memory: mmap() with MADV_SEQUENTIAL

**Sort Algorithm:**
- Parallel quicksort on chunks
- K-way merge of sorted chunks
- Interpreted comparison function
- In-place record swapping

**Data Format:**
- Record size: 16 bytes fixed
- Key 1: 4-byte little-endian uint32 (pos 1, ascending)
- Key 2: 4-byte little-endian uint32 (pos 5, ascending)
- Padding: 8 bytes

## 📊 Benchmark Summary

```
┌─────────────────────────────────────────────┐
│  50 MILLION RECORDS BENCHMARK               │
├─────────────────────────────────────────────┤
│  Records:     50,000,000                    │
│  File Size:   763 MB                        │
│  Sort Time:   24.87s (4 threads)            │
│  Throughput:  30.68 MB/s                    │
│  Rate:        2.01M records/sec             │
│  Speedup:     3.81× over single thread      │
│  Validation:  ✓ PASSED                      │
└─────────────────────────────────────────────┘
```

## 🎓 Conclusions

1. **Excellent Large File Performance:** Successfully sorted 50M records in under 25 seconds

2. **Good Thread Scaling:** 3.81× speedup with 4 threads demonstrates effective parallelization

3. **Consistent Quality:** Performance scales well from thousands to millions of records

4. **Memory Efficient:** Zero-copy memory mapping keeps memory footprint minimal

5. **Production Ready:** Stable, correct, and performant for large-scale data processing

## 🚀 Next Steps

**Potential Improvements:**
1. Implement JIT x64 code generation (currently using interpreted mode)
2. Optimize merge algorithm with priority queue
3. Add SIMD vectorization for comparisons
4. Cache-aware blocking for better locality
5. Radix sort option for integer-heavy keys

**Expected Impact:**
- JIT: 2-5× faster comparisons
- Better merge: 20-30% faster overall
- SIMD: 10-20% improvement on large keys
- Cache optimization: 15-25% on large files

**Current Status:** Production-ready with room for optimization
