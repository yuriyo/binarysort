#pragma once

#include "record.hpp"
#include <string>
#include <vector>
#include <optional>

namespace binsort {

/**
 * Command-line argument parser
 * Syntax: binsort <input> <output> / sort(...) record(...) thread_count(...)
 */
class ArgumentParser {
public:
    struct Arguments {
        std::string input_file;
        std::string output_file;
        std::vector<KeySpec> keys;
        size_t record_length = 0;
        size_t thread_count = 0;  // 0 means auto-detect
    };

    /**
     * Parse command-line arguments
     * @param argc Argument count
     * @param argv Argument values
     * @return Parsed arguments
     * @throws std::runtime_error on parse error
     */
    static Arguments parse(int argc, char* argv[]);

    /**
     * Print usage information
     */
    static void print_usage(const char* program_name);

private:
    /**
     * Parse sort key specification
     * Format: pos,len,type,order[,pos,len,type,order...]
     * Example: 1,4,w,a,5,4,w,d
     */
    static std::vector<KeySpec> parse_sort_spec(const std::string& spec);

    /**
     * Parse a single key type character
     */
    static KeyType parse_key_type(char c);

    /**
     * Parse a single sort order character
     */
    static SortOrder parse_sort_order(char c);

    /**
     * Extract parameter value from format: name(value)
     */
    static std::optional<std::string> extract_param(
        const std::string& arg,
        const std::string& param_name
    );
};

} // namespace binsort
