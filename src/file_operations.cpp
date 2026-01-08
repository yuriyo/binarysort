#include "file_operations.hpp"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <vector>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace binsort {

size_t FileOperations::get_file_size(const std::string& filepath) {
#ifndef _WIN32
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        throw std::runtime_error("Cannot stat file: " + filepath);
    }
    return st.st_size;
#else
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(filepath.c_str(), GetFileExInfoStandard, &fad)) {
        throw std::runtime_error("Cannot get file attributes: " + filepath);
    }
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return static_cast<size_t>(size.QuadPart);
#endif
}

bool FileOperations::file_exists(const std::string& filepath) {
#ifndef _WIN32
    return access(filepath.c_str(), F_OK) == 0;
#else
    return GetFileAttributesA(filepath.c_str()) != INVALID_FILE_ATTRIBUTES;
#endif
}

bool FileOperations::is_same_file(
    const std::string& path1,
    const std::string& path2
) {
#ifndef _WIN32
    struct stat st1, st2;
    if (stat(path1.c_str(), &st1) != 0 || stat(path2.c_str(), &st2) != 0) {
        return false;
    }
    return st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino;
#else
    // Windows: compare canonical paths
    char full1[MAX_PATH], full2[MAX_PATH];
    GetFullPathNameA(path1.c_str(), MAX_PATH, full1, nullptr);
    GetFullPathNameA(path2.c_str(), MAX_PATH, full2, nullptr);
    return _stricmp(full1, full2) == 0;
#endif
}

void FileOperations::copy_file(
    const std::string& src,
    const std::string& dst,
    size_t size
) {
    std::ifstream in(src, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open source file: " + src);
    }
    
    std::ofstream out(dst, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot create destination file: " + dst);
    }
    
    // Copy in chunks
    const size_t chunk_size = 1024 * 1024;  // 1 MB
    std::vector<char> buffer(chunk_size);
    
    size_t remaining = size;
    while (remaining > 0) {
        size_t to_read = std::min(chunk_size, remaining);
        in.read(buffer.data(), to_read);
        
        if (!in && !in.eof()) {
            throw std::runtime_error("Error reading from source file");
        }
        
        size_t actually_read = in.gcount();
        out.write(buffer.data(), actually_read);
        
        if (!out) {
            throw std::runtime_error("Error writing to destination file");
        }
        
        remaining -= actually_read;
    }
}

size_t FileOperations::validate_record_alignment(
    const std::string& filepath,
    size_t record_length
) {
    if (record_length == 0) {
        throw std::invalid_argument("Record length cannot be zero");
    }
    
    size_t file_size = get_file_size(filepath);
    
    if (file_size % record_length != 0) {
        throw std::runtime_error(
            "File size (" + std::to_string(file_size) + 
            ") is not divisible by record length (" + 
            std::to_string(record_length) + ")"
        );
    }
    
    return file_size / record_length;
}

void FileOperations::create_file(
    const std::string& filepath,
    size_t size
) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot create file: " + filepath);
    }
    
    // Seek to size - 1 and write one byte to set file size
    if (size > 0) {
        out.seekp(size - 1);
        out.put(0);
    }
}

} // namespace binsort
