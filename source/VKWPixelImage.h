//
// Created by Zara Hussain on 2023-04-27.
//

#ifndef PIXELENGINE_PIXELIMAGE_H
#define PIXELENGINE_PIXELIMAGE_H

#include "vulkan/vulkan_core.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN // includes vulkan automatically
#include <GLFW/glfw3.h>

#include "PixelImage.h"
#include "Utility.h"

#include <iostream>

class VKWPixelImage {
  public:
    VKWPixelImage(uint32_t width, uint32_t height, bool isSwapChainImage = false, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    VKWPixelImage(const char *imageFile, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    VKWPixelImage() = default;

    // Cleanup
    void cleanUp(PixBackend *devices);

    // create functions
    void createImage(PixBackend *devices, VkImageTiling imageTiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags);
    void createImageView(PixBackend *devices, VkImageAspectFlags aspectFlags);
    void createDepthBufferImage(PixBackend *devices);
    void createTexture(std::string fileName);

    // setter functions
    void setName(std::string name);
    void setImage(VkImage inputImage) { m_image = inputImage; }
    void setImageView(VkImageView inputImageView) { m_imageView = inputImageView; }

    // getter functions
    uint32_t GetWidth() { return m_pixelImage.GetWidth(); }
    uint32_t GetHeight() { return m_pixelImage.GetHeight(); }
    PixelImage getRawImage() { return m_pixelImage; }
    UCHAR* getRawImageData() { return m_pixelImage.GetRawData(); }
    VkImage getImage() { return m_image; }
    VkImageView getImageView() { return m_imageView; }
    VkDeviceMemory getImageDeviceMemory() { return m_imageMemory; }
    VkFormat getFormat();
    VkDeviceSize getImageBufferSize() { return m_pixelImage.GetImageSize(); }
    bool hasBeenInitialized() { return m_ImageInitialized; }
    bool hasBeenCleaned() { return m_ressourcesCleaned; }

    // helper functions

    // loader functions
    void loadTexture(PixBackend *devices, const char* filename);
    void loadEmptyTexture(PixBackend *devices);
    void loadEmptyTexture(PixBackend *devices, uint32_t width, uint32_t height, VkImageUsageFlags flags);

  private:
    PixelImage m_pixelImage{};

    bool m_IsSwapChainImage = false;
    bool m_ImageInitialized = false;
    bool m_ressourcesCleaned = false;

    // vulkan components
    VkFormat m_format{};
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE; // only applicable if PixelImage is used for depth buffer
};

#endif // PIXELENGINE_PIXELIMAGE_H
