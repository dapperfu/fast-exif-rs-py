/* Test loader that always succeeds */
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
    
    /* Always create a simple test image */
    im->w = 200;
    im->h = 200;
    
    if (!__imlib_AllocateData(im)) {
        return LOAD_OOM;
    }
    
    /* Fill with a simple pattern */
    for (int i = 0; i < 200 * 200; i++) {
        im->data[i] = PIXEL_ARGB(0xFF, 0xFF, 0x00, 0x00); /* Red */
    }
    
    return LOAD_SUCCESS;
}

extern "C" int save(ImlibImage *im)
{
    (void)im;
    return 0;
}

IMLIB_LOADER(formats, load, save);
