#ifndef PIXELTYPES_H_
#define PIXELTYPES_H_

#include <cstdint>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "vulkan/vulkan_core.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN //includes vulkan automatically
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

typedef signed char INT8;
typedef short INT16;
typedef int INT32;
typedef long long int64;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;

typedef unsigned char UCHAR;
typedef char CHAR;

class VKWPixelImage; //forward declaration

namespace Pixel {

enum ImageFormat { UNDEFINED, RGB, RGBA };

// this should not change every frame, but can change per individual object/mesh.
// Dynamic Uniform Buffer Object
struct DynamicUBObj {
    glm::mat4 M{};
    glm::mat4 MinvT{};
    int texIndex = -1;
};

// this can change every frame, and can change per individual object/mesh.
// Dynamic Uniform Buffer Object
struct PObj {
    glm::mat4 M{};
    glm::mat4 MinvT{};
};

struct UboVP {
    glm::mat4 V = glm::mat4(1.0f);
    glm::mat4 P = glm::mat4(1.0f);
    glm::vec4 lightPos = glm::vec4(0.0f);
};

// the vertex must only contain member variables of type vec4 (each 16 bytes)
struct Vertex {
    glm::vec4 position{};
    glm::vec4 normal{};
    glm::vec4 color{};
    glm::vec2 texUV{};
};

//vulkan struct component
struct Devices{
    VkPhysicalDevice physicalDevice{};
    VkDevice logicalDevice{};
};

struct PixSwapchain{
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkSwapchainKHR oldSwapchain{VK_NULL_HANDLE};
    VkExtent2D extent{};
    VkFormat format{};
    std::vector<VKWPixelImage> swapchainImages{};
    std::shared_ptr<VKWPixelImage> depthImage;
};

struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentationFamily = -1;
    int computeFamily = -1;

    //check if queue families are valid
    bool isValid() const
    {
        return graphicsFamily >= 0 && presentationFamily >= 0  && computeFamily >= 0;
    }
};

struct SwapchainDetails
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};	//surface properties
    std::vector<VkSurfaceFormatKHR> format;				//color and format
    std::vector<VkPresentModeKHR> presentationMode;		//how image should be presented
};

enum vertexAttributes { POSITION_ATTRIBUTEINDEX, NORMAL_ATTRIBUTEINDEX, COLOR_ATTRIBUTEINDEX, TEXUV_ATTRIBUTEINDEX, ATTRIBUTECOUNT };

} // namespace Pixel

#endif // PIXELTYPES_H_
