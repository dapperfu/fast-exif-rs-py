/* Minimal imlib2 loader skeleton for Nikon NEF High Efficiency star (HE*).
 * Specifically targets HE* stills. Classic NEF should be left to existing loaders.
 * For now, we only detect HE* and report unsupported (decoder pending).
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <Imlib2.h>

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

/* Heuristic: read IFD0 Compression tag (0x0103) to detect non-standard values.
 * Classic NEF often signals baseline/old JPEG; HE/HE* may use vendor/compressed markers.
 * For now, we only detect NEF container and return unsupported to indicate custom loader present. */

int load(ImlibImage *im, ImlibProgressFunction progress, char progress_granularity, char immediate_load)
{
    FILE *fp = fopen(im->file, "rb");
    if (!fp) {
        imlib_context_set_image(im);
        im->loader = NULL;
        return 0;
    }
    uint8_t hdr[16];
    size_t n = fread(hdr, 1, sizeof(hdr), fp);
    if (n < 8 || !is_tiff_magic(hdr, (int)n)) {
        fclose(fp);
        return 0; /* not our format */
    }

    int be = (hdr[0] == 'M');
    uint32_t ifd0 = 0;
    fread(hdr, 1, 4, fp);
    ifd0 = read_u32(hdr, be);
    /* Minimal parse: seek to IFD0 and read number of entries */
    if (fseek(fp, (long)ifd0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    uint8_t tmp[2];
    if (fread(tmp, 1, 2, fp) != 2) { fclose(fp); return 0; }
    int num = read_u16(tmp, be);

    /* Iterate tags to find Compression (0x0103) and basic size */
    int width = 0, height = 0;
    int compression = -1;
    for (int i = 0; i < num; i++) {
        uint8_t ent[12];
        if (fread(ent, 1, 12, fp) != 12) { fclose(fp); return 0; }
        int tag = read_u16(ent, be);
        int type = read_u16(ent + 2, be);
        (void)type;
        uint32_t count = read_u32(ent + 4, be);
        uint32_t val = read_u32(ent + 8, be);
        (void)count;
        if (tag == 0x0100) width = (int)val; /* image width */
        else if (tag == 0x0101) height = (int)val; /* image length */
        else if (tag == 0x0103) compression = (int)val; /* compression */
    }
    /* Only claim files that look like Nikon HE*; otherwise, let other loaders handle. */
    int is_he_star = looks_like_nikon_he_star(fp);
    fclose(fp);
    if (!is_he_star) {
        return 0; /* not our specific HE* variant */
    }

    /* At this point we identified NEF container and HE* marker. Decoder not yet implemented. */
    imlib_context_set_image(im);
    im->w = (width > 0 ? width : 0);
    im->h = (height > 0 ? height : 0);

    /* Signal that format is recognized but not loadable yet */
    imlib_set_error(IMLIB_LOAD_ERROR_UNKNOWN);
    return 0;
}

int save(ImlibImage *im, ImlibProgressFunction progress, char progress_granularity)
{
    (void)im; (void)progress; (void)progress_granularity;
    imlib_set_error(IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT);
    return 0;
}


