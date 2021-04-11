#include "image_loader.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <cstdlib>
#include <cstring>

void ImageLoader::init()
{
    ilInit();
}
void ImageLoader::deinit()
{
    ilShutDown();
}

void ImageLoader::load_image(Image *image, const char *fname, ImageType type)
{
    image->type = type;

    ILuint imgID;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    ilLoadImage(fname);
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    image->width = ilGetInteger(IL_IMAGE_WIDTH);
    image->height = ilGetInteger(IL_IMAGE_HEIGHT);

    s32 imageSize = image->width * image->height * 4;
    image->pixels = (u8*)malloc(imageSize);

    memcpy(image->pixels, ilGetData(), imageSize);

    ilDeleteImages(1, &imgID);
}
void ImageLoader::free_image(Image *image)
{
    free(image->pixels);
}
