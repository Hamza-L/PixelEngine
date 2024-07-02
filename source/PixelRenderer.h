#ifndef PIXELRENDERER_H_
#define PIXELRENDERER_H_

#include "PixelLogger.h"
#include "Utility.h"
#include "PixelTypes.h"

class PixelRenderer {
  public:
    static ErrorLevel Initialize();
    static void Run();
    static const Pixel::Devices GetDevices();

  private:
    static bool s_isInitialized;
};

#endif // PIXELRENDERER_H_
