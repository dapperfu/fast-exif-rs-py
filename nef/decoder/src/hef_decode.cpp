#include "../include/hef_decode.hpp"
#include "../include/hef_bitstream.hpp"
#include <cstring>
#include <algorithm>

using namespace hefraw;

// HE* uses intoPIX TicoRAW - we can see "CONTACT_INTOPIX_" signature
// This is a simplified decoder that attempts to extract basic patterns
bool hefraw::decode_tile_to_cfa16(const uint8_t* bitstream, size_t len,
                                  const TileHeader& th,
                                  std::vector<uint16_t>& out_cfa,
                                  uint32_t stride_px)
{
    if (th.bitDepth != 14) return false;
    const size_t total = (size_t)th.width * (size_t)th.height;
    out_cfa.assign(stride_px * (size_t)th.height, 0);

    // Look for TicoRAW signature
    if (len < 32) return false;
    const char* sig = "CONTACT_INTOPIX_";
    if (memcmp(bitstream + 6, sig, 16) != 0) return false;

    // Skip header and attempt basic bit unpacking
    // TicoRAW typically uses tile-based compression with predictors
    BitReader reader(bitstream + 32, len - 32);
    
    // Simple approach: try to extract 14-bit values assuming some entropy coding
    // This is a placeholder - real TicoRAW reverse engineering would require
    // analyzing the specific entropy coding, predictors, and transforms used
    
    uint32_t pixel_count = 0;
    while (!reader.exhausted() && pixel_count < total) {
        // Try reading 14-bit values with some basic entropy decoding
        uint32_t val = reader.readBits(14);
        if (val > 16383) val = 16383; // clamp to 14-bit
        
        uint32_t row = pixel_count / th.width;
        uint32_t col = pixel_count % th.width;
        if (row < th.height && col < th.width) {
            out_cfa[row * stride_px + col] = (uint16_t)val;
        }
        pixel_count++;
        
        // Skip some bits periodically (entropy coding artifacts)
        if (pixel_count % 64 == 0) {
            reader.readBits(2); // skip alignment bits
        }
    }
    
    return pixel_count > total / 4; // require at least 25% coverage
}

bool hefraw::assemble_image_cfa16(const ImageHeader& ih,
                                  const uint8_t* file_data, size_t file_len,
                                  std::vector<uint16_t>& out_cfa)
{
    if (ih.tiles.empty()) return false;
    out_cfa.assign((size_t)ih.width * (size_t)ih.height, 0);
    
    // Process the main RAW tile (usually the largest)
    const TileHeader* main_tile = nullptr;
    for (const auto& tile : ih.tiles) {
        if (tile.length > 1000000) { // > 1MB likely main RAW
            main_tile = &tile;
            break;
        }
    }
    
    if (!main_tile) return false;
    
    if ((size_t)main_tile->offset + (size_t)main_tile->length > file_len) return false;
    const uint8_t* bs = file_data + main_tile->offset;
    
    std::vector<uint16_t> tile;
    if (!decode_tile_to_cfa16(bs, main_tile->length, *main_tile, tile, ih.width)) return false;
    
    // Copy tile data to output
    for (size_t y = 0; y < ih.height; y++) {
        for (size_t x = 0; x < ih.width; x++) {
            out_cfa[y * ih.width + x] = tile[y * ih.width + x];
        }
    }
    
    return true;
}


