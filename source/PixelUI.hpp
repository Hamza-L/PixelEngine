#ifndef PIXELUI_H_
#define PIXELUI_H_

#include "PixelScene.h"

const int MAX_UI_OBJECTS = 10;
const int MAX_UI_TEXTURE_PER_OBJECT = 16;

class PixelUI : PixelScene {
public:
    PixelUI(PixBackend* backend);
    PixelUI() = default;

private:

};

#endif // PIXELUI_H_
