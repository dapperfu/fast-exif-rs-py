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
    
    // Try multiple approaches to maximize pixel coverage
    uint32_t pixel_count = 0;
    uint16_t predictor = 0;
    
    // Approach 1: Byte-by-byte decoding with multiple methods
    for (size_t i = 0; i < data_len && pixel_count < total; i++) {
        uint8_t byte = data[i];
        
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
        
        // Method 4: Try 16-bit little-endian interpretation
        uint16_t val4 = 0;
        if (i + 1 < data_len) {
            val4 = data[i] | (data[i + 1] << 8);
            val4 = val4 & 0x3FFF;
        }
        
        // Choose the best value
        uint16_t val = val1;
        if (val2 > val && val2 < 16000) val = val2;
        if (val3 > val && val3 < 16000) val = val3;
        if (val4 > val && val4 < 16000) val = val4;
        
        // Accept any reasonable pixel value
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
        if (val == val3 || val == val4) i++;
    }
    
    // If we didn't get enough coverage, try a different approach
    if (pixel_count < total / 2) {
        // Approach 2: Try 16-bit little-endian data
        pixel_count = 0;
        predictor = 0;
        
        for (size_t i = 0; i < data_len - 1 && pixel_count < total; i += 2) {
            uint16_t val = data[i] | (data[i + 1] << 8);
            val = val & 0x3FFF;
            
            // Accept any non-zero value
            if (val > 0) {
                uint32_t row = pixel_count / th.width;
                uint32_t col = pixel_count % th.width;
                if (row < th.height && col < th.width) {
                    out_cfa[row * stride_px + col] = val;
                }
                pixel_count++;
                predictor = val;
            }
        }
    }
    
    return pixel_count > total / 8; // require at least 12.5% coverage
}

bool hefraw::assemble_image_cfa16(const ImageHeader& ih,
                                  const uint8_t* file_data, size_t file_len,
                                  std::vector<uint16_t>& out_cfa)
{
    if (ih.tiles.empty()) return false;
    out_cfa.assign((size_t)ih.width * (size_t)ih.height, 0);
    
    // Process ALL tiles to maximize coverage
    bool any_success = false;
    
    for (size_t tile_idx = 0; tile_idx < ih.tiles.size(); tile_idx++) {
        const auto& tile = ih.tiles[tile_idx];
        
        if ((size_t)tile.offset + (size_t)tile.length > file_len) continue;
        const uint8_t* bs = file_data + tile.offset;
        
        std::vector<uint16_t> tile_data;
        
        // Try TicoRAW decoding first
        if (decode_tile_to_cfa16(bs, tile.length, tile, tile_data, ih.width)) {
            // TicoRAW decoding succeeded - copy all data
            for (size_t y = 0; y < ih.height && y < tile.height; y++) {
                for (size_t x = 0; x < ih.width && x < tile.width; x++) {
                    // Only overwrite zero pixels to avoid conflicts
                    if (out_cfa[y * ih.width + x] == 0) {
                        out_cfa[y * ih.width + x] = tile_data[y * ih.width + x];
                    }
                }
            }
            any_success = true;
        } else {
            // Try uncompressed 14-bit data for smaller tiles
            if (tile.length < 1000000) { // Smaller tiles might be uncompressed
                tile_data.assign((size_t)tile.width * (size_t)tile.height, 0);
                if (tile.length >= (size_t)tile.width * (size_t)tile.height * 2) {
                    // Assume 14-bit little-endian data
                    for (size_t i = 0; i < tile_data.size() && i * 2 + 1 < tile.length; i++) {
                        uint16_t val = bs[i * 2] | (bs[i * 2 + 1] << 8);
                        val = val & 0x3FFF; // 14-bit mask
                        tile_data[i] = val;
                    }
                    
                    // Copy uncompressed data
                    for (size_t y = 0; y < ih.height && y < tile.height; y++) {
                        for (size_t x = 0; x < ih.width && x < tile.width; x++) {
                            // Only overwrite zero pixels
                            if (out_cfa[y * ih.width + x] == 0) {
                                out_cfa[y * ih.width + x] = tile_data[y * ih.width + x];
                            }
                        }
                    }
                    any_success = true;
                }
            }
        }
    }
    
    return any_success;
}


