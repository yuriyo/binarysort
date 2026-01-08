#pragma once

#include "record.hpp"
#include "comparison_generator.hpp"
#include <cstddef>
#include <vector>
#include <thread>

namespace binsort {

/**
 * Multi-threaded parallel sorting engine
 */
class SortEngine {
public:
    struct Config {
        size_t record_length;
        size_t thread_count = std::thread::hardware_concurrency();
        std::vector<KeySpec> keys;
    };

    explicit SortEngine(const Config& config);
    ~SortEngine();

    /**
     * Sort records in-place using parallel algorithm
     * @param data Pointer to the start of record data
     * @param record_count Number of records to sort
     */
    void sort(uint8_t* data, size_t record_count);

    /**
     * Get the comparison function used by this engine
     */
    ComparisonFunc get_comparison_func() const { return compare_func_; }

private:
    Config config_;
    ComparisonFunc compare_func_;
    bool owns_func_;

    struct Chunk {
        uint8_t* start;
        size_t record_count;
    };

    /**
     * Sort a single chunk in a worker thread
     */
    void sort_chunk(Chunk chunk);

    /**
     * Merge sorted chunks
     */
    void merge_chunks(
        uint8_t* data,
        const std::vector<Chunk>& chunks
    );

    /**
     * Helper to swap two records
     */
    void swap_records(uint8_t* a, uint8_t* b) const;
};

/**
 * Quicksort implementation for record data
 */
class RecordQuickSort {
public:
    RecordQuickSort(
        size_t record_length,
        ComparisonFunc compare
    ) : record_length_(record_length)
      , compare_(compare) {}

    void sort(uint8_t* data, size_t record_count);

private:
    size_t record_length_;
    ComparisonFunc compare_;

    void quicksort(uint8_t* data, int64_t low, int64_t high);
    int64_t partition(uint8_t* data, int64_t low, int64_t high);
    void swap_records(uint8_t* a, uint8_t* b);
};

} // namespace binsort
