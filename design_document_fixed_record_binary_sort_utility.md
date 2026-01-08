# Design Document: Fixed-Record Binary Sort Utility

## 1. Purpose and Scope

This document defines the design for a command-line utility that performs high‑performance sorting of fixed‑length record files, similar in function to legacy tools such as **Opttech Sort**. The utility is intended for modern platforms while maintaining deterministic behavior compatible with legacy data layouts.

The primary goals are:
- Support deterministic binary sorting of fixed-length records
- Explicit control over numeric endianness
- Scalable performance via multithreaded sorting
- Script‑friendly, non‑interactive execution

## 2. Target Environment

- Operating Systems: Windows, Linux (portable core)
- Architecture: x86‑64 (little‑endian), portable to big‑endian systems
- Execution mode: Command-line
- Typical usage: Batch jobs, ETL pipelines, CI/CD data preparation

## 3. Functional Requirements

### 3.1 Input / Output

- Accept a single input file and a single output file
- Support in‑place sorting (input and output paths identical)
- Files consist of **fixed‑length records** without delimiters

### 3.2 Record Definition

- Record length must be explicitly specified
- All reads and writes operate on record boundaries only

Example:
```
record(16)
```

### 3.3 Sort Key Definition

The utility shall support **compound sort keys**, defined as ordered tuples of:

```
(start_position, length, type, order)
```

Positions are **1‑based offsets** within the record.

### 3.4 Data Types

The following key types are required:

| Type | Meaning | Endianness |
|-----|--------|------------|
| `c` | Character (byte-wise) | N/A |
| `w` | Binary integer | Little‑endian |
| `W` | Binary integer | Big‑endian |
| `f` | IEEE 754 float | Little‑endian |

Notes:
- `w` and `W` may be 2, 4, or 8 bytes depending on `length`
- Signed comparison is the default

### 3.5 Sort Order

- `a` — Ascending
- `d` — Descending

### 3.6 Multithreading

- Sorting must support parallel execution
- A runtime parameter shall control the number of worker threads

Example:
```
thread_count(8)
```

### 3.7 Stability

- Sorting **does not need to be stable** unless explicitly specified in future extensions

## 4. Non‑Functional Requirements

### 4.1 Performance

- Capable of sorting multi‑GB files
- Linear scaling with CPU cores where memory permits
- Efficient I/O using buffered and memory‑mapped strategies

### 4.2 Determinism

- Given identical inputs and parameters, output must be bit‑for‑bit identical
- Endianness handling must be explicit and platform‑independent

### 4.3 Reliability

- Graceful handling of invalid arguments
- Clear diagnostic messages on failure
- Atomic output replacement for in‑place sorts

## 5. Command-Line Interface

### 5.1 Syntax

```
binsort <input_file> <output_file> \
  / sort(key_spec[,key_spec...]) \
    record(<record_length>) \
    thread_count(<N>)
```

### 5.2 Example

```
binsort linkxys.dat linkxys.dat \
  / sort(1,4,w,a,5,4,w,a,9,4,W,a,13,4,w,a) \
    record(16) \
    thread_count(4)
```

## 6. Internal Architecture

### 6.1 High‑Level Components

1. **Argument Parser**
2. **File Reader / Writer**
3. **Record Abstraction Layer**
4. **Key Extraction Engine**
5. **Multithreaded Sort Engine**
6. **Merge Engine**

### 6.2 Data Flow

1. Read file in record-aligned blocks
2. Partition records into thread‑local chunks
3. Extract sort keys lazily during comparisons
4. Sort chunks in parallel
5. Merge sorted chunks
6. Write output sequentially

## 7. Multithreaded Sorting Strategy

### 7.1 Algorithm

- Use **parallel merge sort** or **parallel radix sort** (for numeric-heavy keys)
- Each thread sorts an independent block of records
- A hierarchical merge phase combines sorted runs

### 7.2 Thread Allocation

- Default: number of logical CPU cores
- User override via `thread_count`
- Minimum: 1

### 7.3 Memory Considerations

- Each thread maintains its own working buffer
- Global memory usage must be bounded and configurable

## 8. Endianness Handling

- Numeric fields are decoded explicitly according to key type
- No reliance on host byte order

Examples:
- `w` → decode as little‑endian
- `W` → decode as big‑endian

This ensures compatibility with:
- Windows‑generated binary files
- Mainframe or network‑ordered data

## 9. Error Handling

- Invalid record length
- File size not divisible by record length
- Overlapping or out‑of‑bounds key definitions
- Unsupported key type or length

Errors must terminate execution with non‑zero exit codes.

## 10. Extensibility Considerations

Future extensions may include:
- Stable sort option
- Locale‑aware character sorting
- Packed decimal support
- External (disk‑based) merge for low‑memory environments

## 11. Testing Strategy

- Unit tests for key decoding (endianness, signedness)
- Deterministic regression tests against known datasets
- Performance benchmarks across thread counts
- Cross‑platform binary compatibility tests

## 12. Security Considerations

- No execution of external commands
- No dynamic code loading
- Input files treated as opaque binary data

---

**Status:** Initial design
**Audience:** Systems engineers, data infrastructure developers
**Intent:** Serve as a foundation for implementation and architectural review

