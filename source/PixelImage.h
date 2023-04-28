//
// Created by Zara Hussain on 2023-04-27.
//

#ifndef PIXELENGINE_PIXELIMAGE_H
#define PIXELENGINE_PIXELIMAGE_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN //includes vulkan automatically
#include <GLFW/glfw3.h>

#include <iostream>
#include "Utility.h"

class PixelImage {
public:
    PixelImage(PixDevice* devices, uint32_t width, uint32_t height, bool isSwapChainImage);
    PixelImage() = default;
    //cleanup
    void cleanUp();

    //create functions
    void createImage(VkImageTiling imageTiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags);

    void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
    void createDepthBufferImage();

    //setter
    void setName(std::string name);
    void setImage(VkImage inputImage){ m_image = inputImage;}
    void setImageView(VkImageView inputImageView){ m_imageView = inputImageView;}

    //getter
    std::string getName();
    VkImage getImage() { return m_image;}
    VkImageView getImageView() {return m_imageView;}

    //helper functions

private:
    VkDevice* m_device{};
    VkPhysicalDevice* m_physicalDevice{};

    //image info
    uint32_t m_width{};
    uint32_t m_height{};
    std::string imageName{};
    bool m_IsSwapChainImage = false;

    //vulkan components
    VkFormat m_format{};
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE; //only applicable if PixelImage is used for depth buffer
};


#endif //PIXELENGINE_PIXELIMAGE_H
