#include "sort_engine.hpp"
#include <algorithm>
#include <execution>
#include <vector>
#include <thread>
#include <cstring>
#include <span>

namespace binsort {

SortEngine::SortEngine(const Config& config)
    : config_(config)
    , compare_func_(nullptr)
    , owns_func_(false) {
    
    // Generate comparison function
    if (ComparisonGenerator::is_available()) {
        compare_func_ = ComparisonGenerator::generate(
            config_.keys,
            config_.record_length
        );
        owns_func_ = true;
    } else {
        compare_func_ = InterpretedComparator::wrap(config_.keys);
        owns_func_ = false;
    }
}

SortEngine::~SortEngine() {
    if (owns_func_ && compare_func_ != nullptr) {
        ComparisonGenerator::free_function(compare_func_);
    }
}

namespace {
    // Generic optimal swap using std::span and stack buffer for small records
    inline void swap_records_optimal(std::span<uint8_t> a, std::span<uint8_t> b) {
        constexpr size_t STACK_BUFFER_SIZE = 256;
        const size_t len = a.size();
        
        if (len <= STACK_BUFFER_SIZE) {
            // Fast path: use stack buffer
            uint8_t temp[STACK_BUFFER_SIZE];
            std::memcpy(temp, a.data(), len);
            std::memcpy(a.data(), b.data(), len);
            std::memcpy(b.data(), temp, len);
        } else {
            // For larger records, use swap_ranges which is optimized by STL
            std::swap_ranges(a.begin(), a.end(), b.begin());
        }
    }
}

void SortEngine::swap_records(uint8_t* a, uint8_t* b) const {
    swap_records_optimal(
        std::span<uint8_t>(a, config_.record_length),
        std::span<uint8_t>(b, config_.record_length)
    );
}

void SortEngine::sort(uint8_t* data, size_t record_count) {
    if (record_count <= 1) return;
    
    const size_t records_per_thread = std::max(
        size_t(1000),  // Minimum chunk size
        record_count / config_.thread_count
    );
    
    // If data is small or single-threaded, use simple quicksort
    if (config_.thread_count == 1 || record_count < records_per_thread * 2) {
        RecordQuickSort sorter(config_.record_length, compare_func_);
        sorter.sort(data, record_count);
        return;
    }
    
    // Parallel sort: partition into chunks
    std::vector<Chunk> chunks;
    size_t offset = 0;
    
    while (offset < record_count) {
        size_t chunk_size = std::min(records_per_thread, record_count - offset);
        chunks.push_back({
            data + offset * config_.record_length,
            chunk_size
        });
        offset += chunk_size;
    }
    
    // Sort each chunk in parallel
    std::vector<std::thread> threads;
    for (auto& chunk : chunks) {
        threads.emplace_back([this, chunk]() {
            RecordQuickSort sorter(config_.record_length, compare_func_);
            sorter.sort(chunk.start, chunk.record_count);
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Merge sorted chunks
    merge_chunks(data, chunks);
}

void SortEngine::merge_chunks(
    uint8_t* data,
    const std::vector<Chunk>& chunks
) {
    if (chunks.size() <= 1) return;
    
    // Simple k-way merge using temporary buffer
    const size_t total_records = [&chunks]() {
        size_t sum = 0;
        for (const auto& chunk : chunks) sum += chunk.record_count;
        return sum;
    }();
    
    std::vector<uint8_t> temp(total_records * config_.record_length);
    
    // Indices for each chunk
    std::vector<size_t> indices(chunks.size(), 0);
    
    // Merge
    for (size_t out = 0; out < total_records; ++out) {
        // Find minimum among chunk heads
        int min_chunk = -1;
        
        for (size_t i = 0; i < chunks.size(); ++i) {
            if (indices[i] >= chunks[i].record_count) continue;
            
            if (min_chunk == -1) {
                min_chunk = i;
            } else {
                const uint8_t* a = chunks[i].start + 
                    indices[i] * config_.record_length;
                const uint8_t* b = chunks[min_chunk].start + 
                    indices[min_chunk] * config_.record_length;
                
                if (compare_func_(a, b) < 0) {
                    min_chunk = i;
                }
            }
        }
        
        // Copy minimum record to output
        const uint8_t* src = chunks[min_chunk].start + 
            indices[min_chunk] * config_.record_length;
        std::memcpy(
            temp.data() + out * config_.record_length,
            src,
            config_.record_length
        );
        indices[min_chunk]++;
    }
    
    // Copy back to original buffer
    std::memcpy(data, temp.data(), temp.size());
}

// QuickSort implementation
void RecordQuickSort::sort(uint8_t* data, size_t record_count) {
    if (record_count <= 1) return;
    quicksort(data, 0, record_count - 1);
}

void RecordQuickSort::quicksort(uint8_t* data, int64_t low, int64_t high) {
    if (low < high) {
        int64_t pi = partition(data, low, high);
        quicksort(data, low, pi - 1);
        quicksort(data, pi + 1, high);
    }
}

int64_t RecordQuickSort::partition(uint8_t* data, int64_t low, int64_t high) {
    uint8_t* pivot = data + high * record_length_;
    int64_t i = low - 1;
    
    for (int64_t j = low; j < high; j++) {
        uint8_t* current = data + j * record_length_;
        if (compare_(current, pivot) < 0) {
            i++;
            swap_records(
                data + i * record_length_,
                current
            );
        }
    }
    
    swap_records(
        data + (i + 1) * record_length_,
        pivot
    );
    
    return i + 1;
}

void RecordQuickSort::swap_records(uint8_t* a, uint8_t* b) {
    swap_records_optimal(
        std::span<uint8_t>(a, record_length_),
        std::span<uint8_t>(b, record_length_)
    );
}

} // namespace binsort
