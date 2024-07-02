#ifndef PIXELFONT_H_
#define PIXELFONT_H_

#include "PixelTypes.h"
#include <iostream>
#include <unordered_map>

#include "ft2build.h"
#include FT_FREETYPE_H


class PixelFont{
    public:
        PixelFont() = default;
        void loadFont(const char* fileName, int* fileSize);
    private:
        static bool isInitialized;
        static std::unordered_map<std::string, FT_Face> s_fontFaces;
};

#endif // PIXELFONT_H_
