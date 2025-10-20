#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace hefraw {

struct TileHeader {
    uint32_t offset;
    uint32_t length;
    uint16_t width;
    uint16_t height;
    uint8_t  bitDepth; // e.g., 14
    uint8_t  cfaPattern; // 0=RGGB etc.
};

struct ImageHeader {
    uint32_t width;
    uint32_t height;
    uint8_t  bitDepth; // 14
    uint8_t  cfaPattern;
    std::vector<TileHeader> tiles;
};

// Parse HE* container structures from a NEF file buffer and populate headers.
// Returns true on success.
bool parse_hef_headers(const uint8_t* data, size_t size, ImageHeader& out);

} // namespace hefraw


