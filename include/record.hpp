#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <span>

namespace binsort {

/**
 * Key type enumeration
 */
enum class KeyType {
    Character,         // 'c' - byte-wise comparison
    LittleEndianInt,   // 'w' - little-endian integer
    BigEndianInt,      // 'W' - big-endian integer
    LittleEndianFloat  // 'f' - IEEE 754 float
};

/**
 * Sort order
 */
enum class SortOrder {
    Ascending,   // 'a'
    Descending   // 'd'
};

/**
 * Key specification (1-based positions as per design doc)
 */
struct KeySpec {
    size_t position;     // 1-based offset within record
    size_t length;       // Length in bytes (2, 4, or 8 for numeric)
    KeyType type;        // Type of the key
    SortOrder order;     // Sort order

    // Convert 1-based position to 0-based offset
    size_t offset() const { return position - 1; }
};

/**
 * Record view - non-owning view into a fixed-length record
 */
class RecordView {
public:
    RecordView(const uint8_t* data, size_t length)
        : data_(data), length_(length) {}

    const uint8_t* data() const { return data_; }
    size_t length() const { return length_; }

    /**
     * Extract a key value for comparison
     * Returns a signed 64-bit integer for all numeric types
     * For character keys, performs byte-wise comparison
     */
    int64_t extract_key(const KeySpec& spec) const;

    /**
     * Extract floating point key
     */
    double extract_float_key(const KeySpec& spec) const;

private:
    const uint8_t* data_;
    size_t length_;

    // Endianness conversion helpers
    static uint16_t read_le16(const uint8_t* ptr);
    static uint32_t read_le32(const uint8_t* ptr);
    static uint64_t read_le64(const uint8_t* ptr);
    static uint16_t read_be16(const uint8_t* ptr);
    static uint32_t read_be32(const uint8_t* ptr);
    static uint64_t read_be64(const uint8_t* ptr);
    static float read_le_float(const uint8_t* ptr);
    static double read_le_double(const uint8_t* ptr);
};

/**
 * Record comparator using multiple keys
 */
class RecordComparator {
public:
    explicit RecordComparator(const std::vector<KeySpec>& keys)
        : keys_(keys) {}

    /**
     * Compare two records
     * Returns: < 0 if a < b, 0 if a == b, > 0 if a > b
     */
    int compare(const RecordView& a, const RecordView& b) const;

    /**
     * Operator for use with std::sort
     */
    bool operator()(const RecordView& a, const RecordView& b) const {
        return compare(a, b) < 0;
    }

    const std::vector<KeySpec>& keys() const { return keys_; }

private:
    std::vector<KeySpec> keys_;
};

/**
 * Record buffer - owns a collection of fixed-length records
 */
class RecordBuffer {
public:
    RecordBuffer(size_t record_length, size_t record_count)
        : record_length_(record_length)
        , data_(record_length * record_count) {}

    uint8_t* data() { return data_.data(); }
    const uint8_t* data() const { return data_.data(); }
    
    size_t record_length() const { return record_length_; }
    size_t record_count() const { return data_.size() / record_length_; }
    size_t size() const { return data_.size(); }

    RecordView get_record(size_t index) const {
        return RecordView(data_.data() + index * record_length_, record_length_);
    }

private:
    size_t record_length_;
    std::vector<uint8_t> data_;
};

} // namespace binsort
