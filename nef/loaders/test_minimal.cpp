/* Minimal test loader to verify imlib2 integration */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <Imlib2.h>
#include <Imlib2_Loader.h>

const char *formats[] = { "nef", NULL };

extern "C" int load(ImlibImage *im, int load_data)
{
    (void)load_data;
    
    /* Check if we have file data */
    if (!im->fi || !im->fi->fdata || im->fi->fsize < 8) {
        return LOAD_FAIL;
    }
    
    const uint8_t *file_data = (const uint8_t*)im->fi->fdata;
    
    /* Check TIFF magic */
    if (file_data[0] != 'I' || file_data[1] != 'I' || 
        file_data[2] != 0x2A || file_data[3] != 0x00) {
        return LOAD_FAIL;
    }
    
    /* Create a simple test image */
    im->w = 100;
    im->h = 100;
    
    if (!__imlib_AllocateData(im)) {
        return LOAD_OOM;
    }
    
    /* Fill with a simple pattern */
    for (int i = 0; i < 100 * 100; i++) {
        im->data[i] = PIXEL_ARGB(0xFF, 0x80, 0x80, 0x80);
    }
    
    return LOAD_SUCCESS;
}

extern "C" int save(ImlibImage *im)
{
    (void)im;
    return 0;
}

IMLIB_LOADER(formats, load, save);
