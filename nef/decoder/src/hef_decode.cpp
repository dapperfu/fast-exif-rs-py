#include "../include/hef_decode.hpp"
#include "../include/hef_bitstream.hpp"
#include <cstring>

using namespace hefraw;

// Placeholder: pass-through bit unpack (no true HE* entropy decode yet)
bool hefraw::decode_tile_to_cfa16(const uint8_t* bitstream, size_t len,
                                  const TileHeader& th,
                                  std::vector<uint16_t>& out_cfa,
                                  uint32_t stride_px)
{
    if (th.bitDepth != 14) return false;
    const size_t total = (size_t)th.width * (size_t)th.height;
    out_cfa.assign(stride_px * (size_t)th.height, 0);

    // NOTE: Actual HE* bitstream decoding TBD. This is a stub that fails.
    (void)bitstream; (void)len; (void)total;
    return false;
}

bool hefraw::assemble_image_cfa16(const ImageHeader& ih,
                                  const uint8_t* file_data, size_t file_len,
                                  std::vector<uint16_t>& out_cfa)
{
    if (ih.tiles.empty()) return false;
    out_cfa.assign((size_t)ih.width * (size_t)ih.height, 0);
    const TileHeader& th = ih.tiles[0];
    if ((size_t)th.offset + (size_t)th.length > file_len) return false;
    const uint8_t* bs = file_data + th.offset;
    std::vector<uint16_t> tile;
    if (!decode_tile_to_cfa16(bs, th.length, th, tile, ih.width)) return false;
    // For single-tile case, tile already placed
    return true;
}


