#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
} Image;

int load_image(const char *filename, Image *img);

#endif
