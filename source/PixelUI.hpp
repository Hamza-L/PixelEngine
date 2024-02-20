#ifndef PIXELUI_H_
#define PIXELUI_H_

#include "PixelScene.h"
#include "glm/fwd.hpp"

const int MAX_UI_OBJECTS = 10;
const int MAX_UI_TEXTURE_PER_OBJECT = 16;

class PixelUI : PixelScene {
public:
    PixelUI() = default;
    PixelUI* addButton(glm::vec2 pos, glm::vec2 size, const char* name);

private:

};

#endif // PIXELUI_H_
