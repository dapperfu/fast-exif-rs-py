#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

namespace hefraw {

class BitReader {
public:
    BitReader(const uint8_t* data, size_t size) noexcept;
    uint32_t readBits(unsigned n) noexcept; // n in [0, 32]
    uint32_t peekBits(unsigned n) const noexcept;
    void alignToByte() noexcept;
    bool exhausted() const noexcept;

private:
    const uint8_t* ptr_;
    const uint8_t* end_;
    uint64_t cache_;
    unsigned bits_; // number of valid bits in cache
    void refill() noexcept;
};

} // namespace hefraw


