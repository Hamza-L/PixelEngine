#ifndef PIXELMEMMANAGER_H_
#define PIXELMEMMANAGER_H_

#include "PixelLogger.h"
#include "PixelTypes.h"
#include "glm/glm.hpp"
#include <vector>

constexpr UINT32 START_MEMORY_ALLOCATION = 1000000000; // 100mb

struct PixelObjectDataHeader {
    unsigned int id;
    int positionOffset;
    int positionSize;
    int normalOffset;
    int normalSize;
    int colorOffset;
    int colorSize;
    int texCoordOffset;
    int texCoordSize;
};

struct PixelImageDataHeader {
    unsigned int id;
};

class PixelMemory {
  private:
    PixelMemory(){};

  public:
    static ErrorLevel InitGlobalMemory();
    static ErrorLevel FreeGlobalMemory();
    static UCHAR *AllocateMemory(const size_t bytesToAllocate);
};

#endif // PIXELMEMMANAGER_H_
