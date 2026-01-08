#include <fstream>
#include <cstdint>
#include <vector>
#include <iostream>
#include <cstring>

// Verify that a file is sorted according to the specified keys

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_to_verify>\n";
        return 1;
    }
    
    std::string input_file = argv[1];
    const size_t record_size = 16;
    
    std::ifstream in(input_file, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open input file\n";
        return 1;
    }
    
    // Get file size
    in.seekg(0, std::ios::end);
    size_t file_size = in.tellg();
    in.seekg(0, std::ios::beg);
    
    if (file_size % record_size != 0) {
        std::cerr << "File size is not a multiple of record size\n";
        return 1;
    }
    
    size_t record_count = file_size / record_size;
    std::cout << "Verifying " << record_count << " records...\n";
    
    std::vector<uint8_t> prev_record(record_size);
    std::vector<uint8_t> curr_record(record_size);
    
    // Read first record
    in.read(reinterpret_cast<char*>(prev_record.data()), record_size);
    
    size_t errors = 0;
    for (size_t i = 1; i < record_count; ++i) {
        in.read(reinterpret_cast<char*>(curr_record.data()), record_size);
        
        // Extract keys (little-endian uint32_t at positions 0 and 4)
        uint32_t prev_key1, prev_key2, curr_key1, curr_key2;
        std::memcpy(&prev_key1, prev_record.data(), 4);
        std::memcpy(&prev_key2, prev_record.data() + 4, 4);
        std::memcpy(&curr_key1, curr_record.data(), 4);
        std::memcpy(&curr_key2, curr_record.data() + 4, 4);
        
        // Check if sorted (ascending on key1, then key2)
        bool out_of_order = false;
        if (curr_key1 < prev_key1) {
            out_of_order = true;
        } else if (curr_key1 == prev_key1 && curr_key2 < prev_key2) {
            out_of_order = true;
        }
        
        if (out_of_order) {
            if (errors < 10) {  // Show first 10 errors
                std::cerr << "Error at record " << i << ": "
                          << "prev=(" << prev_key1 << "," << prev_key2 << ") "
                          << "curr=(" << curr_key1 << "," << curr_key2 << ")\n";
            }
            errors++;
        }
        
        prev_record = curr_record;
    }
    
    if (errors == 0) {
        std::cout << "✓ File is correctly sorted!\n";
        return 0;
    } else {
        std::cerr << "✗ Found " << errors << " ordering errors\n";
        return 1;
    }
}
