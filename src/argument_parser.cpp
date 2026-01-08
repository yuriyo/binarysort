#include "argument_parser.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <thread>

namespace binsort {

ArgumentParser::Arguments ArgumentParser::parse(int argc, char* argv[]) {
    if (argc < 3) {
        throw std::runtime_error("Insufficient arguments");
    }
    
    Arguments args;
    args.input_file = argv[1];
    args.output_file = argv[2];
    
    // Default thread count
    args.thread_count = std::thread::hardware_concurrency();
    if (args.thread_count == 0) args.thread_count = 1;
    
    // Find the "/" separator
    bool found_separator = false;
    for (int i = 3; i < argc; ++i) {
        if (std::string(argv[i]) == "/") {
            found_separator = true;
            
            // Parse parameters after "/"
            for (int j = i + 1; j < argc; ++j) {
                std::string arg = argv[j];
                
                // Check for sort(...)
                if (auto value = extract_param(arg, "sort")) {
                    args.keys = parse_sort_spec(*value);
                }
                // Check for record(...)
                else if (auto value = extract_param(arg, "record")) {
                    args.record_length = std::stoull(*value);
                }
                // Check for thread_count(...)
                else if (auto value = extract_param(arg, "thread_count")) {
                    args.thread_count = std::stoull(*value);
                    if (args.thread_count == 0) args.thread_count = 1;
                }
                else {
                    throw std::runtime_error("Unknown parameter: " + arg);
                }
            }
            break;
        }
    }
    
    if (!found_separator) {
        throw std::runtime_error("Missing '/' separator");
    }
    
    // Validate required parameters
    if (args.keys.empty()) {
        throw std::runtime_error("Missing sort specification");
    }
    if (args.record_length == 0) {
        throw std::runtime_error("Missing or invalid record length");
    }
    
    // Validate key specifications
    for (const auto& key : args.keys) {
        if (key.position == 0) {
            throw std::runtime_error("Key position must be >= 1 (1-based)");
        }
        if (key.offset() + key.length > args.record_length) {
            throw std::runtime_error(
                "Key at position " + std::to_string(key.position) +
                " with length " + std::to_string(key.length) +
                " extends beyond record length " + std::to_string(args.record_length)
            );
        }
        
        // Validate numeric key lengths
        if (key.type != KeyType::Character) {
            if (key.length != 2 && key.length != 4 && key.length != 8) {
                throw std::runtime_error(
                    "Numeric key length must be 2, 4, or 8 bytes"
                );
            }
        }
    }
    
    return args;
}

std::vector<KeySpec> ArgumentParser::parse_sort_spec(const std::string& spec) {
    std::vector<KeySpec> keys;
    std::istringstream ss(spec);
    std::string token;
    
    std::vector<std::string> tokens;
    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        tokens.push_back(token);
    }
    
    // Each key requires 4 tokens: position, length, type, order
    if (tokens.size() % 4 != 0) {
        throw std::runtime_error(
            "Sort specification must have 4 fields per key: position,length,type,order"
        );
    }
    
    for (size_t i = 0; i < tokens.size(); i += 4) {
        KeySpec key;
        
        // Parse position (1-based)
        key.position = std::stoull(tokens[i]);
        
        // Parse length
        key.length = std::stoull(tokens[i + 1]);
        
        // Parse type
        if (tokens[i + 2].length() != 1) {
            throw std::runtime_error("Key type must be a single character");
        }
        key.type = parse_key_type(tokens[i + 2][0]);
        
        // Parse order
        if (tokens[i + 3].length() != 1) {
            throw std::runtime_error("Sort order must be a single character");
        }
        key.order = parse_sort_order(tokens[i + 3][0]);
        
        keys.push_back(key);
    }
    
    return keys;
}

KeyType ArgumentParser::parse_key_type(char c) {
    switch (c) {
        case 'c': return KeyType::Character;
        case 'w': return KeyType::LittleEndianInt;
        case 'W': return KeyType::BigEndianInt;
        case 'f': return KeyType::LittleEndianFloat;
        default:
            throw std::runtime_error(
                std::string("Unknown key type: ") + c
            );
    }
}

SortOrder ArgumentParser::parse_sort_order(char c) {
    switch (c) {
        case 'a': return SortOrder::Ascending;
        case 'd': return SortOrder::Descending;
        default:
            throw std::runtime_error(
                std::string("Unknown sort order: ") + c
            );
    }
}

std::optional<std::string> ArgumentParser::extract_param(
    const std::string& arg,
    const std::string& param_name
) {
    // Check if arg starts with "param_name("
    std::string prefix = param_name + "(";
    if (arg.size() < prefix.size() + 2) return std::nullopt;
    if (arg.substr(0, prefix.size()) != prefix) return std::nullopt;
    if (arg.back() != ')') return std::nullopt;
    
    // Extract value between parentheses
    return arg.substr(prefix.size(), arg.size() - prefix.size() - 1);
}

void ArgumentParser::print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name 
              << " <input_file> <output_file> / <parameters>\n\n"
              << "Parameters:\n"
              << "  sort(pos,len,type,order[,...])\n"
              << "    pos:   1-based position in record\n"
              << "    len:   Length in bytes\n"
              << "    type:  c=character, w=little-endian, W=big-endian, f=float\n"
              << "    order: a=ascending, d=descending\n\n"
              << "  record(length)\n"
              << "    Record length in bytes\n\n"
              << "  thread_count(N)\n"
              << "    Number of threads (default: CPU cores)\n\n"
              << "Example:\n"
              << "  " << program_name 
              << " input.dat output.dat / sort(1,4,w,a,5,4,w,d) record(16) thread_count(4)\n";
}

} // namespace binsort
