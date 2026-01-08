#pragma once

#include <string>
#include <cstddef>

namespace binsort {

/**
 * File operation utilities
 */
class FileOperations {
public:
    /**
     * Get file size in bytes
     */
    static size_t get_file_size(const std::string& filepath);

    /**
     * Check if file exists
     */
    static bool file_exists(const std::string& filepath);

    /**
     * Check if two paths refer to the same file
     */
    static bool is_same_file(
        const std::string& path1,
        const std::string& path2
    );

    /**
     * Copy file from source to destination
     * @param src Source file path
     * @param dst Destination file path
     * @param size Expected size of the file
     */
    static void copy_file(
        const std::string& src,
        const std::string& dst,
        size_t size
    );

    /**
     * Validate that file size is aligned to record length
     * @return Number of records in the file
     * @throws std::runtime_error if not aligned
     */
    static size_t validate_record_alignment(
        const std::string& filepath,
        size_t record_length
    );

    /**
     * Create a new file with specified size (for pre-allocation)
     */
    static void create_file(
        const std::string& filepath,
        size_t size
    );
};

} // namespace binsort
