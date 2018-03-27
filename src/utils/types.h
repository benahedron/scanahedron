#pragma once
#include "defines.h"

SHARED_STRUCT_PTR(RawImage);
/**
 * Basic image buffer
 */
struct RawImage
{
    explicit RawImage(unsigned int width_, unsigned int height_, unsigned int bytesPerPixel_ = 3)
        : width(width_), height(height_), bytesPerPixel(bytesPerPixel_), pixels(new unsigned char[width * height * bytesPerPixel_])
    {
    }

    ~RawImage()
    {
        delete[] pixels;
    }

    unsigned int bytesPerPixel;
    unsigned int width;
    unsigned int height;
    unsigned char *pixels;
};