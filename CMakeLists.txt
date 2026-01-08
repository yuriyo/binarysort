cmake_minimum_required(VERSION 3.25)
project(BinarySort VERSION 1.0 LANGUAGES CXX)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_WINDOWS TRUE)
endif()

# Architecture detection and optimization
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64)")
    set(ARCH_X64 TRUE)
    message(STATUS "Target architecture: x86-64")
else()
    message(WARNING "Non-x64 architecture detected: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Compiler-specific flags for performance
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -O3            # Maximum optimization
        -ffast-math    # Aggressive floating-point optimizations
        -flto          # Link-time optimization
    )
    if(ARCH_X64)
        add_compile_options(-march=native -mavx2 -msse4.2)  # Enable SIMD instructions
    endif()
elseif(MSVC)
    add_compile_options(
        /W4            # Warning level 4
        /O2            # Maximum optimization
        /GL            # Whole program optimization
        /arch:AVX2     # Enable AVX2 instructions
    )
    add_link_options(/LTCG)  # Link-time code generation
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/argument_parser.cpp
    src/memory_mapper.cpp
    src/record.cpp
    src/comparison_generator.cpp
    src/sort_engine.cpp
    src/file_operations.cpp
)

# Platform-specific sources
if(PLATFORM_MACOS OR PLATFORM_LINUX)
    list(APPEND SOURCES src/memory_mapper_unix.cpp)
elseif(PLATFORM_WINDOWS)
    list(APPEND SOURCES src/memory_mapper_windows.cpp)
endif()

# Main executable
add_executable(binsort ${SOURCES})

# Platform-specific libraries
if(PLATFORM_LINUX)
    target_link_libraries(binsort PRIVATE pthread)
endif()

# Enable testing
enable_testing()

# Test executable
add_executable(binsort_test
    tests/test_main.cpp
    tests/test_endianness.cpp
    tests/test_comparison.cpp
    tests/test_memory_mapper.cpp
    src/argument_parser.cpp
    src/memory_mapper.cpp
    src/record.cpp
    src/comparison_generator.cpp
    src/sort_engine.cpp
    src/file_operations.cpp
)

if(PLATFORM_MACOS OR PLATFORM_LINUX)
    target_sources(binsort_test PRIVATE src/memory_mapper_unix.cpp)
elseif(PLATFORM_WINDOWS)
    target_sources(binsort_test PRIVATE src/memory_mapper_windows.cpp)
endif()

if(PLATFORM_LINUX)
    target_link_libraries(binsort_test PRIVATE pthread)
endif()

add_test(NAME BinarySortTests COMMAND binsort_test)

# Installation
install(TARGETS binsort DESTINATION bin)
