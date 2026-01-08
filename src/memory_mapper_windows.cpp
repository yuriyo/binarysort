#include "memory_mapper.hpp"
#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace binsort {

MemoryMapper::MemoryMapper(const std::string& filepath, Mode mode)
    : mode_(mode) {
    map_file(filepath);
}

MemoryMapper::~MemoryMapper() {
    unmap();
}

MemoryMapper::MemoryMapper(MemoryMapper&& other) noexcept
    : data_(other.data_)
    , size_(other.size_)
    , mode_(other.mode_)
    , file_handle_(other.file_handle_)
    , map_handle_(other.map_handle_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.file_handle_ = nullptr;
    other.map_handle_ = nullptr;
}

MemoryMapper& MemoryMapper::operator=(MemoryMapper&& other) noexcept {
    if (this != &other) {
        unmap();
        data_ = other.data_;
        size_ = other.size_;
        mode_ = other.mode_;
        file_handle_ = other.file_handle_;
        map_handle_ = other.map_handle_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.file_handle_ = nullptr;
        other.map_handle_ = nullptr;
    }
    return *this;
}

void MemoryMapper::map_file(const std::string& filepath) {
    // Open file with appropriate access
    DWORD access = (mode_ == Mode::ReadWrite) ? 
        (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
    DWORD share = FILE_SHARE_READ;
    
    file_handle_ = CreateFileA(
        filepath.c_str(),
        access,
        share,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file_handle_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    // Get file size
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle_, &file_size)) {
        CloseHandle(file_handle_);
        throw std::runtime_error("Failed to get file size");
    }
    size_ = static_cast<size_t>(file_size.QuadPart);

    // Create file mapping
    DWORD protect = (mode_ == Mode::ReadWrite) ? 
        PAGE_READWRITE : PAGE_READONLY;
    
    map_handle_ = CreateFileMappingA(
        file_handle_,
        nullptr,
        protect,
        0,
        0,
        nullptr
    );

    if (map_handle_ == nullptr) {
        CloseHandle(file_handle_);
        throw std::runtime_error("Failed to create file mapping");
    }

    // Map view of file
    DWORD map_access = (mode_ == Mode::ReadWrite) ? 
        FILE_MAP_WRITE : FILE_MAP_READ;
    
    data_ = MapViewOfFile(
        map_handle_,
        map_access,
        0,
        0,
        size_
    );

    if (data_ == nullptr) {
        CloseHandle(map_handle_);
        CloseHandle(file_handle_);
        throw std::runtime_error("Failed to map view of file");
    }
}

void MemoryMapper::unmap() {
    if (data_ != nullptr) {
        UnmapViewOfFile(data_);
        data_ = nullptr;
    }
    if (map_handle_ != nullptr) {
        CloseHandle(map_handle_);
        map_handle_ = nullptr;
    }
    if (file_handle_ != nullptr) {
        CloseHandle(file_handle_);
        file_handle_ = nullptr;
    }
    size_ = 0;
}

void MemoryMapper::sync(bool async) {
    if (data_ == nullptr || mode_ != Mode::ReadWrite) {
        return;
    }
    
    if (!FlushViewOfFile(data_, size_)) {
        throw std::runtime_error("Failed to flush memory map");
    }
    
    if (!async) {
        FlushFileBuffers(file_handle_);
    }
}

} // namespace binsort

#endif // _WIN32
