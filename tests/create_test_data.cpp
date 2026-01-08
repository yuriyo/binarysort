#include <fstream>
#include <cstdint>
#include <vector>
#include <random>
#include <iostream>
#include <cstring>

// Create test data file with fixed 16-byte records
// Format: [4 bytes: uint32_t key1][4 bytes: uint32_t key2][8 bytes: padding]

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <output_file> <record_count>\n";
        return 1;
    }
    
    std::string output_file = argv[1];
    size_t record_count = std::stoull(argv[2]);
    
    const size_t record_size = 16;
    std::vector<uint8_t> buffer(record_size);
    
    std::ofstream out(output_file, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to create output file\n";
        return 1;
    }
    
    std::random_device rd;
    std::mt19937 gen(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<uint32_t> dist(0, 1000);
    
    for (size_t i = 0; i < record_count; ++i) {
        // Generate random keys
        uint32_t key1 = dist(gen);
        uint32_t key2 = dist(gen);
        
        // Write as little-endian
        std::memcpy(buffer.data(), &key1, 4);
        std::memcpy(buffer.data() + 4, &key2, 4);
        
        // Padding (zeros)
        std::memset(buffer.data() + 8, 0, 8);
        
        out.write(reinterpret_cast<const char*>(buffer.data()), record_size);
    }
    
    std::cout << "Created " << output_file << " with " << record_count 
              << " records (" << (record_count * record_size) << " bytes)\n";
    
    return 0;
}
