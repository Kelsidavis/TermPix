#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"
#include "image.h"
#include <stdio.h>

extern int silent_mode;

int load_image(const char *filename, Image *img) {
    if (!silent_mode) {
        printf("Attempting to load: %s\n", filename);
    }

    // Force 3 channels (RGB) for consistency
    img->data = stbi_load(filename, &img->width, &img->height, &img->channels, 3);
    if (!img->data) {
        fprintf(stderr, "stbi_load failed for '%s': %s\n", filename, stbi_failure_reason());
        return 0;
    }

    // We manually set channels to 3 since we asked for 3
    img->channels = 3;
    return 1;
}