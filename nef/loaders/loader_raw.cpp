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
    
    /* Debug the header and file data */
    FILE *debug_fp = fopen("/tmp/debug_decode", "w");
    if (debug_fp) {
        fprintf(debug_fp, "Decode Debug Info:\n");
        fprintf(debug_fp, "  File size: %zu\n", file_size);
        fprintf(debug_fp, "  Header width: %u\n", header.width);
        fprintf(debug_fp, "  Header height: %u\n", header.height);
        fprintf(debug_fp, "  First 16 bytes of file: ");
        for (int i = 0; i < 16 && i < (int)file_size; i++) {
            fprintf(debug_fp, "%02x ", file_data[i]);
        }
        fprintf(debug_fp, "\n");
        fclose(debug_fp);
    }
    
    if (!assemble_image_cfa16(header, file_data, file_size, cfa)) {
        return LOAD_BADIMAGE;
    }
    
    /* Check if CFA decoding succeeded */
    if (debug_fp) {
        debug_fp = fopen("/tmp/debug_decode", "a");
        if (debug_fp) {
            fprintf(debug_fp, "  CFA size: %zu\n", cfa.size());
            fprintf(debug_fp, "  Expected size: %u\n", header.width * header.height);
            fclose(debug_fp);
        }
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
    
    /* Debug CFA data quality and check for patterns */
    uint16_t min_val = 0xFFFF, max_val = 0;
    uint32_t zero_count = 0, non_zero_count = 0;
    
    for (int i = 0; i < (int)(header.width * header.height); i++) {
        if (cfa[i] < min_val) min_val = cfa[i];
        if (cfa[i] > max_val) max_val = cfa[i];
        if (cfa[i] == 0) zero_count++;
        else non_zero_count++;
    }
    
    /* Write debug info to file */
    debug_fp = fopen("/tmp/cfa_debug", "w");
    if (debug_fp) {
        fprintf(debug_fp, "CFA Debug Info:\n");
        fprintf(debug_fp, "  Width: %u\n", header.width);
        fprintf(debug_fp, "  Height: %u\n", header.height);
        fprintf(debug_fp, "  Min value: %u\n", min_val);
        fprintf(debug_fp, "  Max value: %u\n", max_val);
        fprintf(debug_fp, "  Range: %u\n", max_val - min_val);
        fprintf(debug_fp, "  Zero pixels: %u\n", zero_count);
        fprintf(debug_fp, "  Non-zero pixels: %u\n", non_zero_count);
        fprintf(debug_fp, "  First 20 values: ");
        for (int i = 0; i < 20 && i < (int)(header.width * header.height); i++) {
            fprintf(debug_fp, "%u ", cfa[i]);
        }
        fprintf(debug_fp, "\n");
        
        /* Check if there's a pattern in the first few rows */
        fprintf(debug_fp, "First row values: ");
        for (int x = 0; x < 20 && x < (int)header.width; x++) {
            fprintf(debug_fp, "%u ", cfa[x]);
        }
        fprintf(debug_fp, "\n");
        
        fclose(debug_fp);
    }
    
    /* Implement proper CFA demosaicing (Bayer pattern RGGB) */
    for (int y = 0; y < (int)header.height; y++) {
        for (int x = 0; x < (int)header.width; x++) {
            int idx = y * header.width + x;
            uint16_t val = cfa[idx];
            
            /* Convert with proper scaling based on actual range */
            uint8_t pixel;
            if (max_val > min_val) {
                pixel = ((val - min_val) * 255) / (max_val - min_val);
            } else {
                pixel = 0;
            }
            
            /* Simple Bayer demosaicing - assume RGGB pattern */
            uint8_t r, g, b;
            
            if ((y % 2) == 0 && (x % 2) == 0) {
                // Red pixel
                r = pixel;
                g = pixel; // Use same value for now
                b = pixel; // Use same value for now
            } else if ((y % 2) == 0 && (x % 2) == 1) {
                // Green pixel
                r = pixel; // Use same value for now
                g = pixel;
                b = pixel; // Use same value for now
            } else if ((y % 2) == 1 && (x % 2) == 0) {
                // Green pixel
                r = pixel; // Use same value for now
                g = pixel;
                b = pixel; // Use same value for now
            } else {
                // Blue pixel
                r = pixel; // Use same value for now
                g = pixel; // Use same value for now
                b = pixel;
            }
            
            imlib_data[idx] = PIXEL_ARGB(0xFF, r, g, b);
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
