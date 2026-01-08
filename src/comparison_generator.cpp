#include "comparison_generator.hpp"
#include <cstring>
#include <stdexcept>

#ifndef _WIN32
#include <sys/mman.h>
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace binsort {

// Check if JIT is available (x64 only for now)
bool ComparisonGenerator::is_available() {
#if defined(__x86_64__) || defined(_M_X64)
    return true;
#else
    return false;
#endif
}

ComparisonGenerator::CodeBuffer::~CodeBuffer() {
    if (memory != nullptr) {
#ifndef _WIN32
        munmap(memory, capacity);
#else
        VirtualFree(memory, 0, MEM_RELEASE);
#endif
    }
}

void ComparisonGenerator::ensure_capacity(CodeBuffer& code, size_t required) {
    if (code.capacity == 0) {
        // Initial allocation
        code.capacity = (required < 4096) ? 4096 : required;
        
#ifndef _WIN32
        code.memory = mmap(
            nullptr,
            code.capacity,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );
        if (code.memory == MAP_FAILED) {
            throw std::runtime_error("Failed to allocate code buffer");
        }
#else
        code.memory = VirtualAlloc(
            nullptr,
            code.capacity,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
        );
        if (code.memory == nullptr) {
            throw std::runtime_error("Failed to allocate code buffer");
        }
#endif
    }
    else if (code.size + required > code.capacity) {
        throw std::runtime_error("Code buffer overflow");
    }
}

void ComparisonGenerator::emit_byte(CodeBuffer& code, uint8_t byte) {
    ensure_capacity(code, 1);
    static_cast<uint8_t*>(code.memory)[code.size++] = byte;
}

void ComparisonGenerator::emit_bytes(CodeBuffer& code, const void* bytes, size_t count) {
    ensure_capacity(code, count);
    std::memcpy(
        static_cast<uint8_t*>(code.memory) + code.size,
        bytes,
        count
    );
    code.size += count;
}

void ComparisonGenerator::make_executable(CodeBuffer& code) {
#ifndef _WIN32
    if (mprotect(code.memory, code.capacity, PROT_READ | PROT_EXEC) != 0) {
        throw std::runtime_error("Failed to make code executable");
    }
#else
    DWORD old_protect;
    if (!VirtualProtect(code.memory, code.capacity, PAGE_EXECUTE_READ, &old_protect)) {
        throw std::runtime_error("Failed to make code executable");
    }
#endif
}

void ComparisonGenerator::emit_prologue(CodeBuffer& code) {
    // x64 System V ABI: rdi = a, rsi = b
    // x64 Windows ABI: rcx = a, rdx = b
    
#ifdef _WIN32
    // Windows: move rcx -> rdi, rdx -> rsi for consistency
    uint8_t prologue[] = {
        0x48, 0x89, 0xcf,  // mov rdi, rcx
        0x48, 0x89, 0xd6,  // mov rsi, rdx
    };
#else
    // Unix: parameters already in rdi, rsi
    uint8_t prologue[] = {
        0x55,              // push rbp
        0x48, 0x89, 0xe5,  // mov rbp, rsp
    };
#endif
    
    emit_bytes(code, prologue, sizeof(prologue));
}

void ComparisonGenerator::emit_epilogue(CodeBuffer& code) {
    // Return value in eax
#ifndef _WIN32
    uint8_t epilogue[] = {
        0x5d,              // pop rbp
        0xc3,              // ret
    };
#else
    uint8_t epilogue[] = {
        0xc3,              // ret
    };
#endif
    
    emit_bytes(code, epilogue, sizeof(epilogue));
}

void ComparisonGenerator::emit_key_comparison(
    [[maybe_unused]] CodeBuffer& code,
    [[maybe_unused]] const KeySpec& spec,
    [[maybe_unused]] size_t record_length
) {
    // This is a simplified version - full implementation would generate
    // optimized assembly for each key type
    
    // For now, this is a placeholder showing the structure
    // Real implementation would emit:
    // 1. Load values from records at specified offsets
    // 2. Perform endianness conversion if needed
    // 3. Compare values
    // 4. Jump to next key if equal, return if not
}

ComparisonFunc ComparisonGenerator::generate(
    const std::vector<KeySpec>& keys,
    size_t record_length
) {
    if (!is_available()) {
        // Fall back to interpreted comparison
        return InterpretedComparator::wrap(keys);
    }

    CodeBuffer code;
    
    try {
        emit_prologue(code);
        
        // Generate comparison code for each key
        for (const auto& key : keys) {
            emit_key_comparison(code, key, record_length);
        }
        
        // If all keys equal, return 0
        // mov eax, 0
        uint8_t ret_zero[] = {0x31, 0xc0};  // xor eax, eax
        emit_bytes(code, ret_zero, sizeof(ret_zero));
        
        emit_epilogue(code);
        make_executable(code);
        
        // Return function pointer (ownership transferred)
        ComparisonFunc func = reinterpret_cast<ComparisonFunc>(code.memory);
        code.memory = nullptr;  // Prevent destructor from freeing
        return func;
    }
    catch (...) {
        // Fall back to interpreted version on error
        return InterpretedComparator::wrap(keys);
    }
}

void ComparisonGenerator::free_function(ComparisonFunc func) {
    if (func == nullptr) return;
    
    // For now, we don't track generated functions
    // In production, we'd need a registry to know the size
#ifndef _WIN32
    // Cannot free without knowing size
#else
    VirtualFree(reinterpret_cast<void*>(func), 0, MEM_RELEASE);
#endif
}

// Interpreted comparator implementation
int InterpretedComparator::compare(const uint8_t* a, const uint8_t* b) const {
    RecordView va(a, 1000000);  // Size doesn't matter for comparison
    RecordView vb(b, 1000000);
    
    RecordComparator cmp(keys_);
    return cmp.compare(va, vb);
}

// Global storage for interpreted comparator instance
static InterpretedComparator* g_comparator = nullptr;

static int interpreted_compare_wrapper(const uint8_t* a, const uint8_t* b) {
    if (g_comparator == nullptr) {
        return 0;  // Should never happen
    }
    return g_comparator->compare(a, b);
}

ComparisonFunc InterpretedComparator::wrap(const std::vector<KeySpec>& keys) {
    // Note: This leaks memory intentionally since the comparator must live
    // as long as the function pointer is in use
    g_comparator = new InterpretedComparator(keys);
    return interpreted_compare_wrapper;
}

} // namespace binsort
