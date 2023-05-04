//
// Created by Zara Hussain on 2023-04-27.
//

#ifndef PIXELENGINE_UTILITY_H
#define PIXELENGINE_UTILITY_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN //includes vulkan automatically
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

//vulkan struct component
struct PixDevice{
    VkPhysicalDevice physicalDevice{};
    VkDevice logicalDevice{};
};

struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentationFamily = -1;

    //check if queue families are valid
    bool isValid() const
    {
        return graphicsFamily >= 0 && presentationFamily >= 0;
    }
};

struct SwapchainDetails
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};	//surface properties
    std::vector<VkSurfaceFormatKHR> format;				//color and format
    std::vector<VkPresentModeKHR> presentationMode;		//how image should be presented
};

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags)
{
    //Get properties from physical device
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    for(uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
    {
        if( (allowedTypes & (1 << i)) &&
            ((deviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) )
        {
            return i;
        }
    }

    return 0;
}

static bool checkInstanceExtensionSupport(const std::vector<const char*>* checkExtensions)
{
    //get the number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    //create a vector of the size of extensions and populate it
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for (const auto &checkExtension: *checkExtensions)
    {
        bool hasExtension = false;
        for (const auto& extension : extensions)
        {
            if (strcmp(checkExtension, extension.extensionName) == 0)
            {
                hasExtension = true;
                break;
            }
        }

        if (!hasExtension)
        {
            return false;
        }

    }

    return true;

}

static bool checkInstanceLayerSupport(const std::vector<const char*>* checkLayers)
{
    //get the number of extensions
    uint32_t LayerCount = 0;
    vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

    //create a vector of the size of extensions and populate it
    std::vector<VkLayerProperties> layers(LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, layers.data());

    for (const auto &checkLayer: *checkLayers)
    {
        bool hasLayer = false;
        for (const auto& layer : layers)
        {
            if (strcmp(checkLayer, layer.layerName) == 0)
            {
                hasLayer = true;
                break;
            }
        }

        if (!hasLayer)
        {
            return false;
        }

    }

    return true;

}

static std::vector<const char*> getRequiredExtensions()
{
    std::vector<const char*> extensions;

    // Set up glfw extensions instance will use
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    //add glfw extensions to list of extentions
    for (size_t i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    return extensions;
}

//best format is subjective but ours is:
// format: VK_FORMAT_R8G8B8A8_UNORM (backup : VK_FORMAT_B8G8R8A8_UNORM)
// colorspace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
static VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        return{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& format : formats)
    {
        if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];
}


static VkFormat chooseSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &formats, VkImageTiling imageTiling,
                                                     VkFormatFeatureFlags featureFlags) {

    //loop through options and find compatible one

    for(VkFormat format : formats)
    {
        //get properties
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

        //depending on tiling choice, need to return
        if(imageTiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
        else if (imageTiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & featureFlags) == featureFlags)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format for given image tiling");

    //return VK_NULL_HANDLE;
}

static VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes)
{
    for (const auto& presentationMode : presentationModes)
    {
        if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentationMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR; //this is always available so if the desired present mode is not found, we return FIFO Present mode
}

#endif //PIXELENGINE_UTILITY_H

