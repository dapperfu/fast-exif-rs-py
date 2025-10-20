// stb_image - v2.29 - public domain image loader - http://nothings.org/stb
// This is a trimmed header for JPEG-only decoding use in this project.
// For full license and latest version, see upstream repository.

#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char *stbi_load_from_memory(const unsigned char *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
extern void stbi_image_free(void *retval_from_stbi_load);

#ifdef __cplusplus
}
#endif

#endif /* STB_IMAGE_H */

// Implementation section (JPEG-only minimal) will be compiled in loader_nef.c via STB_IMAGE_IMPLEMENTATION.

