#pragma once

#include "record.hpp"
#include <functional>
#include <memory>
#include <cstdint>

namespace binsort {

/**
 * Function signature for generated comparison function
 * Returns: < 0 if a < b, 0 if a == b, > 0 if a > b
 */
using ComparisonFunc = int(*)(const uint8_t* a, const uint8_t* b);

/**
 * JIT comparison function generator
 * Generates x64 machine code for efficient record comparison
 * This avoids DSL parsing overhead during sorting
 */
class ComparisonGenerator {
public:
    /**
     * Generate a comparison function for the given key specifications
     * @param keys Vector of key specifications
     * @param record_length Length of each record
     * @return Function pointer to the generated comparison code
     */
    static ComparisonFunc generate(
        const std::vector<KeySpec>& keys,
        size_t record_length
    );

    /**
     * Check if JIT code generation is available on this platform
     */
    static bool is_available();

    /**
     * Free the memory allocated for a generated function
     */
    static void free_function(ComparisonFunc func);

private:
    struct CodeBuffer {
        void* memory = nullptr;
        size_t size = 0;
        size_t capacity = 0;

        ~CodeBuffer();
    };

    // Emit x64 assembly instructions
    static void emit_prologue(CodeBuffer& code);
    static void emit_epilogue(CodeBuffer& code);
    static void emit_key_comparison(
        CodeBuffer& code,
        const KeySpec& spec,
        size_t record_length
    );
    static void emit_byte(CodeBuffer& code, uint8_t byte);
    static void emit_bytes(CodeBuffer& code, const void* bytes, size_t count);
    static void ensure_capacity(CodeBuffer& code, size_t required);
    static void make_executable(CodeBuffer& code);
};

/**
 * Fallback interpreter-based comparator
 * Used when JIT is not available or as a reference implementation
 */
class InterpretedComparator {
public:
    explicit InterpretedComparator(const std::vector<KeySpec>& keys)
        : keys_(keys) {}

    int compare(const uint8_t* a, const uint8_t* b) const;

    static ComparisonFunc wrap(const std::vector<KeySpec>& keys);

private:
    std::vector<KeySpec> keys_;
};

} // namespace binsort
