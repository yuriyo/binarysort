#include "record.hpp"
#include <cstring>
#include <bit>
#include <stdexcept>

namespace binsort {

// Endianness conversion helpers
uint16_t RecordView::read_le16(const uint8_t* ptr) {
    uint16_t value;
    std::memcpy(&value, ptr, sizeof(value));
    if constexpr (std::endian::native == std::endian::big) {
        return __builtin_bswap16(value);
    }
    return value;
}

uint32_t RecordView::read_le32(const uint8_t* ptr) {
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(value));
    if constexpr (std::endian::native == std::endian::big) {
        return __builtin_bswap32(value);
    }
    return value;
}

uint64_t RecordView::read_le64(const uint8_t* ptr) {
    uint64_t value;
    std::memcpy(&value, ptr, sizeof(value));
    if constexpr (std::endian::native == std::endian::big) {
        return __builtin_bswap64(value);
    }
    return value;
}

uint16_t RecordView::read_be16(const uint8_t* ptr) {
    uint16_t value;
    std::memcpy(&value, ptr, sizeof(value));
    if constexpr (std::endian::native == std::endian::little) {
        return __builtin_bswap16(value);
    }
    return value;
}

uint32_t RecordView::read_be32(const uint8_t* ptr) {
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(value));
    if constexpr (std::endian::native == std::endian::little) {
        return __builtin_bswap32(value);
    }
    return value;
}

uint64_t RecordView::read_be64(const uint8_t* ptr) {
    uint64_t value;
    std::memcpy(&value, ptr, sizeof(value));
    if constexpr (std::endian::native == std::endian::little) {
        return __builtin_bswap64(value);
    }
    return value;
}

float RecordView::read_le_float(const uint8_t* ptr) {
    uint32_t bits = read_le32(ptr);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

double RecordView::read_le_double(const uint8_t* ptr) {
    uint64_t bits = read_le64(ptr);
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

int64_t RecordView::extract_key(const KeySpec& spec) const {
    const uint8_t* ptr = data_ + spec.offset();
    
    // Validate bounds
    if (spec.offset() + spec.length > length_) {
        throw std::out_of_range("Key extends beyond record boundary");
    }

    switch (spec.type) {
        case KeyType::Character: {
            // For character keys, we need byte-wise comparison
            // Return the first byte as signed for now
            // Full comparison will be done in the comparator
            return static_cast<int8_t>(ptr[0]);
        }
        
        case KeyType::LittleEndianInt: {
            int64_t value = 0;
            switch (spec.length) {
                case 2:
                    value = static_cast<int16_t>(read_le16(ptr));
                    break;
                case 4:
                    value = static_cast<int32_t>(read_le32(ptr));
                    break;
                case 8:
                    value = static_cast<int64_t>(read_le64(ptr));
                    break;
                default:
                    throw std::invalid_argument("Invalid integer length");
            }
            return value;
        }
        
        case KeyType::BigEndianInt: {
            int64_t value = 0;
            switch (spec.length) {
                case 2:
                    value = static_cast<int16_t>(read_be16(ptr));
                    break;
                case 4:
                    value = static_cast<int32_t>(read_be32(ptr));
                    break;
                case 8:
                    value = static_cast<int64_t>(read_be64(ptr));
                    break;
                default:
                    throw std::invalid_argument("Invalid integer length");
            }
            return value;
        }
        
        case KeyType::LittleEndianFloat:
            // Float comparison is handled separately
            return 0;
    }
    
    return 0;
}

double RecordView::extract_float_key(const KeySpec& spec) const {
    const uint8_t* ptr = data_ + spec.offset();
    
    if (spec.type != KeyType::LittleEndianFloat) {
        throw std::invalid_argument("Not a float key");
    }
    
    switch (spec.length) {
        case 4:
            return static_cast<double>(read_le_float(ptr));
        case 8:
            return read_le_double(ptr);
        default:
            throw std::invalid_argument("Invalid float length");
    }
}

int RecordComparator::compare(const RecordView& a, const RecordView& b) const {
    for (const auto& key : keys_) {
        int cmp = 0;
        
        if (key.type == KeyType::Character) {
            // Byte-wise comparison
            const uint8_t* pa = a.data() + key.offset();
            const uint8_t* pb = b.data() + key.offset();
            cmp = std::memcmp(pa, pb, key.length);
        }
        else if (key.type == KeyType::LittleEndianFloat) {
            // Floating-point comparison
            double va = a.extract_float_key(key);
            double vb = b.extract_float_key(key);
            if (va < vb) cmp = -1;
            else if (va > vb) cmp = 1;
            else cmp = 0;
        }
        else {
            // Integer comparison
            int64_t va = a.extract_key(key);
            int64_t vb = b.extract_key(key);
            if (va < vb) cmp = -1;
            else if (va > vb) cmp = 1;
            else cmp = 0;
        }
        
        // Apply sort order
        if (key.order == SortOrder::Descending) {
            cmp = -cmp;
        }
        
        // If not equal, return comparison result
        if (cmp != 0) {
            return cmp;
        }
    }
    
    // All keys are equal
    return 0;
}

} // namespace binsort
