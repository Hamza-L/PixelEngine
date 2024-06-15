#ifndef PIXELFONT_H_
#define PIXELFONT_H_

#include "PixelTypes.h"

class PixelFont{
    public:
        PixelFont() = default;
        uint8_t* loadFont(const char* fileName, int* fileSize);
};

#endif // PIXELFONT_H_
