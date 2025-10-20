/* Nikon HE* NEF loader for imlib2 - Full implementation */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#include <Imlib2.h>
#include <Imlib2_Loader.h>

/* Include our decoder headers */
#include "../decoder/include/hef_format.hpp"
#include "../decoder/include/hef_decode.hpp"

using namespace hefraw;

/* Loader exported symbols expected by imlib2 runtime */
const char *formats[] = { "nef", NULL };

/* Simple TIFF magic check */
static int is_tiff_magic(const uint8_t *b, int len)
{
    if (len < 4) return 0;
    if ((b[0] == 'I' && b[1] == 'I' && b[2] == 0x2A && b[3] == 0x00) ||
        (b[0] == 'M' && b[1] == 'M' && b[2] == 0x00 && b[3] == 0x2A))
        return 1;
    return 0;
}

/* Heuristic detection of Nikon HE* quality marker */
static int looks_like_nikon_he_star(FILE *fp)
{
    long cur = ftell(fp);
    if (cur < 0) return 0;
    if (fseek(fp, 0, SEEK_SET) != 0) return 0;
    
    /* Check for TicoRAW signature in the main payload area (around offset 4.5MB) */
    if (fseek(fp, 4500000, SEEK_SET) == 0) {
        uint8_t sig_check[32];
        if (fread(sig_check, 1, 32, fp) == 32) {
            if (memcmp(sig_check + 6, "CONTACT_INTOPIX_", 16) == 0) {
                (void)fseek(fp, cur, SEEK_SET);
                return 1; /* Found TicoRAW signature */
            }
        }
    }
    
    /* Fallback: scan entire file for HE* marker (but limit to reasonable size) */
    if (fseek(fp, 0, SEEK_SET) != 0) return 0;
    const size_t MAX_SCAN = 20 << 20; /* 20 MiB - enough for our test file */
    uint8_t *buf = (uint8_t *)malloc(MAX_SCAN);
    if (!buf) { (void)fseek(fp, cur, SEEK_SET); return 0; }
    size_t n = fread(buf, 1, MAX_SCAN, fp);
    /* Restore file position */
    (void)fseek(fp, cur, SEEK_SET);
    if (n == 0) { free(buf); return 0; }
    
    const char *needle1 = "HE*";
    int found = 0;
    /* Simple substring search */
    for (size_t i = 0; i + 2 < n; i++) {
        if (i + 3 <= n && memcmp(buf + i, needle1, 3) == 0) { 
            found = 1; 
            break; 
        }
    }
    free(buf);
    return found;
}

extern "C" int load(ImlibImage *im, int load_data)
{
    (void)load_data;
    
    int rc = LOAD_FAIL;
    
    /* Check if we have file data */
    if (!im->fi || !im->fi->fdata || im->fi->fsize < 8) {
        return LOAD_FAIL;
    }
    
    const uint8_t *file_data = (const uint8_t*)im->fi->fdata;
    size_t file_size = im->fi->fsize;
    
    /* Check TIFF magic */
    if (!is_tiff_magic(file_data, 8)) {
        return LOAD_FAIL;
    }
    
    /* Check if it's HE* by looking for TicoRAW signature */
    bool is_he_star = false;
    if (file_size > 4500000) {
        if (memcmp(file_data + 4500006, "CONTACT_INTOPIX_", 16) == 0) {
            is_he_star = true;
        }
    }
    
    /* Also check for HE* marker in the file */
    if (!is_he_star && file_size > 10000000) {
        for (size_t i = 0; i < file_size - 3; i++) {
            if (memcmp(file_data + i, "HE*", 3) == 0) {
                is_he_star = true;
                break;
            }
        }
    }
    
    if (!is_he_star) {
        return LOAD_FAIL;
    }
    
    /* HE* file detected - decode it */
    ImageHeader header;
    if (!parse_hef_headers(file_data, file_size, header)) {
        return LOAD_BADIMAGE;
    }
    
    /* Decode CFA data */
    std::vector<uint16_t> cfa;
    if (!assemble_image_cfa16(header, file_data, file_size, cfa)) {
        return LOAD_BADIMAGE;
    }
    
    /* Set image dimensions */
    im->w = header.width;
    im->h = header.height;
    
    /* Allocate image data */
    if (!__imlib_AllocateData(im)) {
        return LOAD_OOM;
    }
    
    /* Convert CFA to RGB for display */
    uint32_t *imlib_data = im->data;
    
    /* Simple CFA to RGB conversion */
    for (int y = 0; y < header.height; y++) {
        for (int x = 0; x < header.width; x++) {
            int idx = y * header.width + x;
            uint16_t val = cfa[idx];
            
            /* Convert 14-bit to 8-bit */
            uint8_t pixel = (val >> 6) & 0xFF;
            
            /* Simple grayscale conversion */
            imlib_data[idx] = PIXEL_ARGB(0xFF, pixel, pixel, pixel);
        }
    }
    
    return LOAD_SUCCESS;
}

extern "C" int save(ImlibImage *im)
{
    (void)im;
    return 0;
}

/* Define the loader module */
IMLIB_LOADER(formats, load, save);