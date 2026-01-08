#include "memory_mapper.hpp"
#include <stdexcept>
#include <cstring>

#ifndef _WIN32
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

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
    , fd_(other.fd_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.fd_ = -1;
}

MemoryMapper& MemoryMapper::operator=(MemoryMapper&& other) noexcept {
    if (this != &other) {
        unmap();
        data_ = other.data_;
        size_ = other.size_;
        mode_ = other.mode_;
        fd_ = other.fd_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.fd_ = -1;
    }
    return *this;
}

void MemoryMapper::map_file(const std::string& filepath) {
    // Open file with appropriate flags
    int flags = (mode_ == Mode::ReadWrite) ? O_RDWR : O_RDONLY;
    fd_ = open(filepath.c_str(), flags);
    
    if (fd_ == -1) {
        throw std::runtime_error("Failed to open file: " + filepath + 
                                 " - " + std::strerror(errno));
    }

    // Get file size
    struct stat st;
    if (fstat(fd_, &st) == -1) {
        close(fd_);
        throw std::runtime_error(std::string("Failed to get file size: ") + 
                                 std::strerror(errno));
    }
    size_ = st.st_size;

    // Map the file
    int prot = (mode_ == Mode::ReadWrite) ? (PROT_READ | PROT_WRITE) : PROT_READ;
    int map_flags = MAP_SHARED;
    
    data_ = mmap(nullptr, size_, prot, map_flags, fd_, 0);
    
    if (data_ == MAP_FAILED) {
        close(fd_);
        data_ = nullptr;
        throw std::runtime_error(std::string("Failed to map file: ") + 
                                 std::strerror(errno));
    }

    // Advise kernel about access pattern (sequential)
    madvise(data_, size_, MADV_SEQUENTIAL);
}

void MemoryMapper::unmap() {
    if (data_ != nullptr) {
        munmap(data_, size_);
        data_ = nullptr;
    }
    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
    size_ = 0;
}

void MemoryMapper::sync(bool async) {
    if (data_ == nullptr || mode_ != Mode::ReadWrite) {
        return;
    }
    
    int flags = async ? MS_ASYNC : MS_SYNC;
    if (msync(data_, size_, flags) == -1) {
        throw std::runtime_error("Failed to sync memory map: " + 
                                 std::string(std::strerror(errno)));
    }
}

} // namespace binsort

#endif // !_WIN32
