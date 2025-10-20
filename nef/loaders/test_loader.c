/* Minimal test loader to verify imlib2 integration */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <Imlib2.h>
#include <Imlib2_Loader.h>

const char *formats[] = { "nef", NULL };

int load(ImlibImage *im, int load_data)
{
    (void)im;
    (void)load_data;
    
    /* Always return 0 for now - just test if loader is called */
    return 0;
}

int save(ImlibImage *im)
{
    (void)im;
    return 0;
}

IMLIB_LOADER(formats, load, save);
