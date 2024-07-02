#include "PixelImage.h"
#include "PixelMemManager.h"

using namespace Pixel;

PixelImage::PixelImage(const UINT32 width, const UINT32 height, ImageFormat format)
    : m_width(width), m_height(height), m_format(format), m_imageName("untitled"), m_fileName("") {

    if(format == 1)
        m_numChannels = 1;//cases where we want to create an empty image

    m_numChannels = format == ImageFormat::RGB ? 3 : 4;
    m_dataSize = width * height * m_numChannels;

}

//cases where we want to create an empty image
PixelImage::PixelImage()
    : m_width(1), m_height(1), m_format(ImageFormat::UNDEFINED), m_imageName("untitled"), m_fileName("") {

    m_numChannels = 1;
    m_dataSize = 1;

}

void PixelImage::_cleanUp(){

}

PixelImage::PixelImage(const char *imageFile) {}

void PixelImage::LoadImageFile(const char* filePath) {
    int channels, width, height;

    stbi_uc *data_ptr = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);

    if (!data_ptr) {
        throw std::runtime_error("Failed to load texture file: " + std::string(filePath));
        return;
    }

    m_width = (int)width;
    m_height = (int)height;
    m_numChannels = 4;
    m_format = ImageFormat::RGBA; //Most gpu vendors default to RGBA. RGB is not always supported so always default to RGB
    m_dataSize = m_width * m_height * m_numChannels;

    m_data = PixelMemory::AllocateMemory(m_dataSize);
    memcpy(m_data, data_ptr, m_dataSize);
    stbi_image_free(data_ptr);
}

void PixelImage::LoadEmptyImage(const UINT32 width, const UINT32 height) {

    m_width = (int)width;
    m_height = (int)height;
    m_numChannels = 4;
    m_dataSize = width * height * m_numChannels;
    m_format = ImageFormat::RGBA;
}

void PixelImage::FillImage(const std::vector<float> &color) {}
