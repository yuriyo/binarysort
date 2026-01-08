#include "argument_parser.hpp"
#include "file_operations.hpp"
#include "memory_mapper.hpp"
#include "sort_engine.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace binsort;

int main(int argc, char* argv[]) {
    try {
        // Parse arguments
        auto args = ArgumentParser::parse(argc, argv);
        
        std::cout << "Binary Sort Utility\n";
        std::cout << "===================\n";
        std::cout << "Input:        " << args.input_file << "\n";
        std::cout << "Output:       " << args.output_file << "\n";
        std::cout << "Record size:  " << args.record_length << " bytes\n";
        std::cout << "Keys:         " << args.keys.size() << "\n";
        std::cout << "Threads:      " << args.thread_count << "\n";
        
        // Validate input file
        if (!FileOperations::file_exists(args.input_file)) {
            throw std::runtime_error("Input file does not exist: " + args.input_file);
        }
        
        // Validate record alignment
        size_t record_count = FileOperations::validate_record_alignment(
            args.input_file,
            args.record_length
        );
        
        std::cout << "Records:      " << record_count << "\n";
        std::cout << "\n";
        
        // Check if in-place sorting
        bool in_place = FileOperations::is_same_file(args.input_file, args.output_file);
        
        std::string sort_file = args.output_file;
        
        if (!in_place) {
            // Copy input to output first
            std::cout << "Copying input to output...\n";
            auto start = std::chrono::high_resolution_clock::now();
            
            size_t file_size = FileOperations::get_file_size(args.input_file);
            FileOperations::copy_file(args.input_file, args.output_file, file_size);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Copy completed in " << duration.count() << " ms\n\n";
        } else {
            std::cout << "In-place sorting detected\n\n";
        }
        
        // Map the file for sorting (read-write mode)
        std::cout << "Mapping file into memory...\n";
        MemoryMapper mapper(sort_file, MemoryMapper::Mode::ReadWrite);
        
        std::cout << "Mapped " << mapper.size() << " bytes\n\n";
        
        // Create sort engine
        SortEngine::Config config;
        config.record_length = args.record_length;
        config.thread_count = args.thread_count;
        config.keys = args.keys;
        
        SortEngine engine(config);
        
        // Perform sort
        std::cout << "Sorting...\n";
        auto start = std::chrono::high_resolution_clock::now();
        
        engine.sort(static_cast<uint8_t*>(mapper.data()), record_count);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Sort completed in " << duration.count() << " ms\n";
        
        // Calculate throughput
        double seconds = duration.count() / 1000.0;
        double mb_per_sec = (mapper.size() / (1024.0 * 1024.0)) / seconds;
        
        std::cout << "Throughput: " << std::fixed << std::setprecision(2) 
                  << mb_per_sec << " MB/s\n\n";
        
        // Sync changes to disk
        std::cout << "Syncing to disk...\n";
        mapper.sync(false);
        
        std::cout << "Done!\n";
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        ArgumentParser::print_usage(argv[0]);
        return 1;
    }
}
