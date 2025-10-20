#include "../include/hef_bitstream.hpp"

using namespace hefraw;

BitReader::BitReader(const uint8_t* data, size_t size) noexcept
    : ptr_(data), end_(data + size), cache_(0), bits_(0) {}

void BitReader::refill() noexcept {
    while (bits_ <= 56 && ptr_ < end_) {
        cache_ |= (uint64_t)(*ptr_++) << bits_;
        bits_ += 8;
    }
}

uint32_t BitReader::peekBits(unsigned n) const noexcept {
    if (n == 0) return 0;
    if (n > 32) n = 32;
    uint64_t mask = (n == 64) ? ~0ull : ((1ull << n) - 1ull);
    return (uint32_t)(cache_ & mask);
}

uint32_t BitReader::readBits(unsigned n) noexcept {
    if (n == 0) return 0;
    if (n > 32) n = 32;
    if (bits_ < n) refill();
    uint32_t v = (uint32_t)(cache_ & ((1ull << n) - 1ull));
    cache_ >>= n;
    bits_ = (unsigned)(bits_ >= n ? bits_ - n : 0);
    return v;
}

void BitReader::alignToByte() noexcept {
    unsigned drop = bits_ & 7u ? (bits_ & 7u) : 0u;
    if (drop) { cache_ >>= drop; bits_ -= drop; }
}

bool BitReader::exhausted() const noexcept { return ptr_ >= end_ && bits_ == 0; }


