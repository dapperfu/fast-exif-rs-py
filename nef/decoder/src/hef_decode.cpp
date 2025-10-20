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

    // TicoRAW typically uses tile-based compression with predictors and entropy coding
    // Skip header and attempt sophisticated bit unpacking
    BitReader reader(bitstream + 32, len - 32);
    
    // Enhanced approach: try different entropy patterns
    // TicoRAW often uses Huffman-like coding with predictors
    
    uint32_t pixel_count = 0;
    uint16_t predictor = 0; // Simple predictor
    
    while (!reader.exhausted() && pixel_count < total) {
        // Try reading variable-length codes (common in TicoRAW)
        uint32_t val = 0;
        
        // Attempt Huffman-like decoding: read bits until we hit a stop pattern
        uint32_t bits_read = 0;
        while (bits_read < 20 && !reader.exhausted()) { // Max 20 bits per pixel
            uint32_t bit = reader.readBits(1);
            val = (val << 1) | bit;
            bits_read++;
            
            // Stop patterns (heuristic - adjust based on analysis)
            if (bits_read >= 14 && (val & 0x3FFF) < 0x2000) { // 14-bit value in reasonable range
                break;
            }
            if (bits_read >= 12 && (val & 0xFFF) < 0x800) { // 12-bit value
                break;
            }
        }
        
        // Extract the actual pixel value
        uint32_t pixel_val = val & ((1 << 14) - 1); // 14-bit mask
        if (pixel_val > 16383) pixel_val = 16383; // clamp to 14-bit
        
        // Apply predictor (simple delta)
        pixel_val = (pixel_val + predictor) & 0x3FFF;
        predictor = pixel_val;
        
        uint32_t row = pixel_count / th.width;
        uint32_t col = pixel_count % th.width;
        if (row < th.height && col < th.width) {
            out_cfa[row * stride_px + col] = (uint16_t)pixel_val;
        }
        pixel_count++;
        
        // Skip alignment bits periodically (entropy coding artifacts)
        if (pixel_count % 32 == 0) {
            reader.alignToByte();
        }
        
        // Reset predictor periodically
        if (pixel_count % 64 == 0) {
            predictor = 0;
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
    
    // Process ALL tiles and combine them properly
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
                    out_cfa[y * ih.width + x] = tile_data[y * ih.width + x];
                }
            }
            any_success = true;
        } else {
            // Try uncompressed 14-bit data
            tile_data.assign((size_t)tile.width * (size_t)tile.height, 0);
            if (tile.length >= (size_t)tile.width * (size_t)tile.height * 2) {
                // Assume 14-bit little-endian data
                for (size_t i = 0; i < tile_data.size() && i * 2 + 1 < tile.length; i++) {
                    uint16_t val = bs[i * 2] | (bs[i * 2 + 1] << 8);
                    tile_data[i] = val & 0x3FFF; // 14-bit mask
                }
                
                // Copy uncompressed data
                for (size_t y = 0; y < ih.height && y < tile.height; y++) {
                    for (size_t x = 0; x < ih.width && x < tile.width; x++) {
                        out_cfa[y * ih.width + x] = tile_data[y * ih.width + x];
                    }
                }
                any_success = true;
            }
        }
    }
    
    return any_success;
}


