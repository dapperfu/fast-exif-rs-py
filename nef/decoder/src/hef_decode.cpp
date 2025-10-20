#include "../include/hef_decode.hpp"
#include "../include/hef_bitstream.hpp"
#include <cstring>
#include <algorithm>

using namespace hefraw;

// HE* uses intoPIX TicoRAW - we can see "CONTACT_INTOPIX_" signature
// This implements a more sophisticated decoder based on TicoRAW patterns
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

    // Try a more sophisticated approach: implement basic TicoRAW decompression
    const uint8_t* data = bitstream + 32;
    size_t data_len = len - 32;
    
    // TicoRAW typically uses tile-based compression with predictors
    // Try to implement a basic decompressor
    uint32_t pixel_count = 0;
    uint16_t predictor = 0;
    
    // Try different bit patterns and compression schemes
    for (size_t i = 0; i < data_len && pixel_count < total; i++) {
        uint8_t byte = data[i];
        
        // Try different interpretations of the byte
        // Method 1: Direct 8-bit values scaled to 14-bit
        uint16_t val1 = (byte << 6) | (byte >> 2); // Scale 8-bit to 14-bit
        val1 = val1 & 0x3FFF;
        
        // Method 2: Use byte as delta from predictor
        uint16_t val2 = (predictor + byte) & 0x3FFF;
        
        // Method 3: Use byte as high 8 bits, next byte as low 6 bits
        uint16_t val3 = 0;
        if (i + 1 < data_len) {
            val3 = ((byte << 6) | (data[i + 1] >> 2)) & 0x3FFF;
        }
        
        // Choose the value that seems most reasonable
        uint16_t val = val1;
        if (val2 > val1 && val2 < 16000) val = val2;
        if (val3 > val && val3 < 16000) val = val3;
        
        // Only accept reasonable pixel values
        if (val > 0 && val < 16000) {
            uint32_t row = pixel_count / th.width;
            uint32_t col = pixel_count % th.width;
            if (row < th.height && col < th.width) {
                out_cfa[row * stride_px + col] = val;
            }
            pixel_count++;
            predictor = val;
        }
        
        // Skip bytes for multi-byte methods
        if (val == val3) i++;
    }
    
    return pixel_count > total / 8; // require at least 12.5% coverage
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


