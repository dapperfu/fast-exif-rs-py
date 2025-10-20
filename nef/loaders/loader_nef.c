/* Minimal imlib2 loader skeleton for Nikon NEF High Efficiency star (HE*).
 * Specifically targets HE* stills. Classic NEF should be left to existing loaders.
 * For now, we only detect HE* and report unsupported (decoder pending).
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <Imlib2.h>
#include <Imlib2_Loader.h>

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

/* Helper functions for TIFF parsing - currently unused but kept for future expansion */
#if 0
static int read_u16(const uint8_t *p, int be)
{
    return be ? (p[0] << 8) | p[1] : (p[1] << 8) | p[0];
}

static uint32_t read_u32(const uint8_t *p, int be)
{
    if (be)
        return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
    return ((uint32_t)p[3] << 24) | ((uint32_t)p[2] << 16) | ((uint32_t)p[1] << 8) | (uint32_t)p[0];
}
#endif

/* Heuristic detection of Nikon HE* quality marker.
 * We conservatively scan an upper-bound slice for the ASCII token "HE*" or
 * the phrase "High Efficiency*" which appears in some MakerNote/XMP payloads.
 */
static int looks_like_nikon_he_star(FILE *fp)
{
    long cur = ftell(fp);
    if (cur < 0) return 0;
    if (fseek(fp, 0, SEEK_SET) != 0) return 0;
    const size_t MAX_SCAN = 1 << 20; /* 1 MiB */
    uint8_t *buf = (uint8_t *)malloc(MAX_SCAN);
    if (!buf) { (void)fseek(fp, cur, SEEK_SET); return 0; }
    size_t n = fread(buf, 1, MAX_SCAN, fp);
    /* Restore file position */
    (void)fseek(fp, cur, SEEK_SET);
    if (n == 0) { free(buf); return 0; }
    const char *needle1 = "HE*";
    const char *needle2 = "High Efficiency*";
    int found = 0;
    /* Simple substring search */
    for (size_t i = 0; i + 2 < n; i++) {
        if (!found && i + 3 <= n && memcmp(buf + i, needle1, 3) == 0) { found = 1; break; }
        if (!found && i + 16 <= n && memcmp(buf + i, needle2, 16) == 0) { found = 1; break; }
    }
    free(buf);
    return found;
}

int load(ImlibImage *im, int load_data)
{
    (void)im;      /* Suppress unused parameter warning */
    (void)load_data; /* Suppress unused parameter warning */
    
    const char *filename = imlib_image_get_filename();
    if (!filename) return 0;
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return 0;
    }
    uint8_t hdr[16];
    size_t n = fread(hdr, 1, sizeof(hdr), fp);
    if (n < 8 || !is_tiff_magic(hdr, (int)n)) {
        fclose(fp);
        return 0; /* not our format */
    }

    /* Only claim files that look like Nikon HE*; otherwise, let other loaders handle. */
    int is_he_star = looks_like_nikon_he_star(fp);
    fclose(fp);
    
    /* If not HE*, bail early */
    if (!is_he_star) { 
        return 0; 
    }

    /* For now, we detect HE* but don't load it (decoder integration pending) */
    /* This allows the loader to be present but defer to other loaders */
    return 0;
}

int save(ImlibImage *im)
{
    (void)im;
    return 0;
}

/* Define the loader module */
IMLIB_LOADER(formats, load, save);