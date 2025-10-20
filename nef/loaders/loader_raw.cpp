/* RAW loader for Nikon HE* NEF files */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

extern "C" {
#include <Imlib2.h>
#include <Imlib2_Loader.h>
}

/* Include our decoder headers */
#include "../decoder/include/hef_format.hpp"
#include "../decoder/include/hef_decode.hpp"

using namespace hefraw;

static const char *const formats[] = { "raw", "arw", "cr2", "dcr", "dng", "nef", "orf", "raf", "rw2", "rwl", "srw" };

/* Simple TIFF magic check */
static int is_tiff_magic(const uint8_t *b, int len)
{
    if (len < 4) return 0;
    if ((b[0] == 'I' && b[1] == 'I' && b[2] == 0x2A && b[3] == 0x00) ||
        (b[0] == 'M' && b[1] == 'M' && b[2] == 0x00 && b[3] == 0x2A))
        return 1;
    return 0;
}

extern "C" {

static int load(ImlibImage *im, int load_data)
{
    (void)load_data;
    
    /* DEBUG: Write file to prove we're being called */
    FILE *fp = fopen("/tmp/raw_loader_called", "w");
    if (fp) {
        fprintf(fp, "CALLED\n");
        fclose(fp);
    }
    fprintf(stderr, "RAW LOADER CALLED!\n");
    
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
    
    /* Simple grayscale conversion first to test CFA data quality */
    for (int y = 0; y < (int)header.height; y++) {
        for (int x = 0; x < (int)header.width; x++) {
            int idx = y * header.width + x;
            uint16_t val = cfa[idx];
            
            /* Convert 14-bit to 8-bit with better scaling */
            uint8_t pixel = (val * 255) / 16383;  /* Scale to 0-255 range */
            
            /* Simple grayscale for now to verify CFA data */
            imlib_data[idx] = PIXEL_ARGB(0xFF, pixel, pixel, pixel);
        }
    }
    
    return LOAD_SUCCESS;
}

static int save(ImlibImage *im)
{
    (void)im;
    return 0;
}

IMLIB_LOADER(formats, load, save);

}
