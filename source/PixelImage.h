#ifndef PIXELIMAGE_H_
#define PIXELIMAGE_H_

#include "PixelTypes.h"

class PixelImage {
public:
    PixelImage(unsigned int width, unsigned int height, PixelImageFormat format = RGBA);
    PixelImage(const char* imageFile);
    PixelImage() = default;
};

#endif // PIXELIMAGE_H_
