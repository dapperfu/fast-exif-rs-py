#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include "hef_format.hpp"

namespace hefraw {

// Decode a single HE* tile into a 16-bit CFA buffer (RGGB order assumed).
// Returns true on success.
bool decode_tile_to_cfa16(const uint8_t* bitstream, size_t len,
                          const TileHeader& th,
                          std::vector<uint16_t>& out_cfa,
                          uint32_t stride_px);

// Assemble full image CFA from tiles into 16-bit buffer of size width*height.
bool assemble_image_cfa16(const ImageHeader& ih,
                          const uint8_t* file_data, size_t file_len,
                          std::vector<uint16_t>& out_cfa);

} // namespace hefraw


