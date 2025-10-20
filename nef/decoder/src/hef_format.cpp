#include "../include/hef_format.hpp"
#include <cstring>

using namespace hefraw;

static bool is_tiff(const uint8_t* d, size_t n, bool& be, uint32_t& ifd0)
{
    if (n < 8) return false;
    if (d[0]=='I'&&d[1]=='I'&&d[2]==0x2A&&d[3]==0x00) { be=false; }
    else if (d[0]=='M'&&d[1]=='M'&&d[2]==0x00&&d[3]==0x2A) { be=true; }
    else return false;
    auto rd32 = [&](const uint8_t* p){
        return be ? (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|uint32_t(p[3])
                  : (uint32_t(p[3])<<24)|(uint32_t(p[2])<<16)|(uint32_t(p[1])<<8)|uint32_t(p[0]); };
    ifd0 = rd32(d+4);
    return ifd0 < n;
}

bool hefraw::parse_hef_headers(const uint8_t* data, size_t size, ImageHeader& out)
{
    bool be=false; uint32_t ifd0=0; if (!is_tiff(data,size,be,ifd0)) return false;
    auto rd16 = [&](const uint8_t* p){ return be ? (p[0]<<8)|p[1] : (p[1]<<8)|p[0]; };
    auto rd32 = [&](const uint8_t* p){ return be ? (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|uint32_t(p[3]) : (uint32_t(p[3])<<24)|(uint32_t(p[2])<<16)|(uint32_t(p[1])<<8)|uint32_t(p[0]); };
    if (ifd0 + 2 > size) return false;
    const uint8_t* p = data + ifd0;
    int num = rd16(p); p += 2;
    uint32_t width=0, height=0, sub_ifds=0;
    for (int i=0;i<num;i++) {
        if (p + 12 > data + size) return false;
        int tag = rd16(p+0);
        int type = rd16(p+2); (void)type;
        uint32_t count = rd32(p+4); (void)count;
        uint32_t val = rd32(p+8);
        if (tag == 0x0100) width = val;
        else if (tag == 0x0101) height = val;
        else if (tag == 0x014A) sub_ifds = val;
        p += 12;
    }
    
    // If width/height not in IFD0, look in SubIFDs
    if ((width==0 || height==0) && sub_ifds && sub_ifds + 24 <= size) {
        for (int sub_idx = 0; sub_idx < 6; sub_idx++) {
            uint32_t sub_off = rd32(data + sub_ifds + 4 * sub_idx);
            if (sub_off && sub_off + 2 <= size) {
                const uint8_t* sp = data + sub_off;
                int sn = rd16(sp); sp += 2;
                for (int i=0;i<sn;i++) {
                    if (sp + 12 > data + size) break;
                    int tag = rd16(sp+0);
                    int type = rd16(sp+2); (void)type;
                    uint32_t count = rd32(sp+4); (void)count;
                    uint32_t val = rd32(sp+8);
                    if (tag == 0x0100 && width == 0) width = val;
                    else if (tag == 0x0101 && height == 0) height = val;
                    sp += 12;
                }
                if (width > 0 && height > 0) break;
            }
        }
    }
    
    if (width==0||height==0) return false;
    out.width = width; out.height = height; out.bitDepth = 14; out.cfaPattern = 0; // defaults

    // Parse all SubIFDs to find tiles with valid StripOffsets/StripByteCounts
    if (sub_ifds && sub_ifds + 24 <= size) { // 6 SubIFDs * 4 bytes each
        for (int sub_idx = 0; sub_idx < 6; sub_idx++) {
            uint32_t sub_off = rd32(data + sub_ifds + 4 * sub_idx);
            if (sub_off && sub_off + 2 <= size) {
                const uint8_t* sp = data + sub_off;
                int sn = rd16(sp); sp += 2;
                uint32_t so=0, sbc=0;
                for (int i=0;i<sn;i++) {
                    if (sp + 12 > data + size) break;
                    int tag = rd16(sp+0);
                    int type = rd16(sp+2); (void)type;
                    uint32_t count = rd32(sp+4); (void)count;
                    uint32_t val = rd32(sp+8);
                    if (tag == 0x0111) so = val;
                    else if (tag == 0x0117) sbc = val;
                    sp += 12;
                }
                if (so && sbc && so + sbc <= size) {
                    TileHeader th{};
                    th.offset = so; th.length = sbc; th.width = (uint16_t)width; th.height=(uint16_t)height; th.bitDepth=14; th.cfaPattern=0;
                    out.tiles.push_back(th);
                }
            }
        }
    }
    return !out.tiles.empty(); // require at least one tile
}


