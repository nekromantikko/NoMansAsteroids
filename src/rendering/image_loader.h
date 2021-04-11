#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H
#include "../util/typedef.h"
#include "rendering_util.h"

namespace ImageLoader
{
    void init();
    void deinit();

    void load_image(Image *image, const char *fname, ImageType type = IMAGE_SRGB);
    void free_image(Image *image);
}

#endif // IMAGE_LOADER_H
