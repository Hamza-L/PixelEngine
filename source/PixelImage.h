#ifndef PIXELIMAGE_H_
#define PIXELIMAGE_H_

#include <iostream>

#include "stb_image.h"

#include "PixelMemManager.h"
#include "PixelTypes.h"

#include <vector>

constexpr UINT16 MAX_IMAGE_NAME_LENGTH = 256;

using namespace Pixel;

class PixelImage {
  public:
    PixelImage(const UINT32 width, const UINT32 height, ImageFormat format = RGBA);
    PixelImage(const char *imageFile);
    PixelImage();

    void LoadImageFile(const char *filePath);
    void LoadEmptyImage(const UINT32 width, const UINT32 height);
    void FillImage(const std::vector<float> &color);

    // getter
    UINT32 GetWidth() { return m_width; }
    UINT32 GetHeight() { return m_height; }
    UINT64 GetImageSize() { return m_dataSize; }
    ImageFormat GetFormat() { return m_format; }
    UCHAR *GetRawData() { return m_data; }
    std::string GetName() { return m_imageName; }

    // cleanup
    void _cleanUp();

  private:
    UCHAR *m_data{};
    UINT32 m_dataSize{};

    // image info
    UINT32 m_width{};
    UINT32 m_height{};
    UINT32 m_numChannels{};
    std::string m_imageName{};
    std::string m_fileName{};
    ImageFormat m_format{};

    // state
    bool isDefined = false;
};

#endif // PIXELIMAGE_H_
