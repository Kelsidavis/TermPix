// render.h
#ifndef RENDER_H
#define RENDER_H

#include "image.h"
extern int enable_dithering;
extern int render_mode;
void render_image(const Image *img, int max_width, int max_height);

#endif
