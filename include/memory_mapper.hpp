#pragma once

#include <cstddef>
#include <string>
#include <memory>

namespace binsort {

/**
 * Cross-platform memory mapping abstraction
 * Supports both read-only and read-write modes
 */
class MemoryMapper {
public:
    enum class Mode {
        ReadOnly,
        ReadWrite
    };

    /**
     * Map a file into memory
     * @param filepath Path to the file
     * @param mode Mapping mode (ReadOnly or ReadWrite)
     * @throws std::runtime_error on failure
     */
    MemoryMapper(const std::string& filepath, Mode mode);
    
    /**
     * Unmap the file
     */
    ~MemoryMapper();

    // Non-copyable
    MemoryMapper(const MemoryMapper&) = delete;
    MemoryMapper& operator=(const MemoryMapper&) = delete;

    // Movable
    MemoryMapper(MemoryMapper&& other) noexcept;
    MemoryMapper& operator=(MemoryMapper&& other) noexcept;

    /**
     * Get pointer to the mapped memory
     */
    void* data() noexcept { return data_; }
    const void* data() const noexcept { return data_; }

    /**
     * Get the size of the mapped region
     */
    size_t size() const noexcept { return size_; }

    /**
     * Sync changes to disk (for ReadWrite mode)
     * @param async If true, return immediately; if false, wait for completion
     */
    void sync(bool async = false);

private:
    void* data_ = nullptr;
    size_t size_ = 0;
    Mode mode_;
    
    // Platform-specific handle
#ifdef _WIN32
    void* file_handle_ = nullptr;
    void* map_handle_ = nullptr;
#else
    int fd_ = -1;
#endif

    void map_file(const std::string& filepath);
    void unmap();
};

} // namespace binsort
