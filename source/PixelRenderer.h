#pragma once

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include "PixelComputePipeline.h"
#include "PixelGraphicsPipeline.h"
#include "PixelWindow.h"
#include "Utility.h"

#include <iostream>
#include <memory>
#include <vector>

const int MAX_FRAME_DRAWS = 2; // we always have "MAX_FRAME_DRAWS" being drawing at once.
static glm::uvec2 mouseCoord = {0, 0};
static glm::uvec2 lastClicked = {0, 0};

class PixelRenderer {
  public:
    PixelRenderer() = default;
    PixelRenderer(const PixelRenderer &) = delete;

    PixelRenderer &operator=(const PixelRenderer &) = delete;
    ~PixelRenderer() = default;

    int initRenderer();
    void addScene(PixelScene *pixScene);
    void draw();
    void run();
    bool windowShouldClose();
    void cleanup();

    float currentTime = 0;

  private:
    // vulkan component
#ifdef __APPLE__
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset", "VK_EXT_descriptor_indexing"};
#else
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif

    // logical and physical device
    PixBackend mainDevice{};

    // swapchain component
    PixSwapchain m_pixSwapchain{};

    // window component
    PixelWindow pixWindow{};

    // physical device features the logical device will be using
    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkInstance m_instance{};
    VkQueue graphicsQueue{};
    VkQueue presentationQueue{};
    VkQueue computeQueue{};
    VkSurfaceKHR m_surface{};
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;
    std::vector<std::unique_ptr<PixelGraphicsPipeline>> graphicsPipelines;
    std::unique_ptr<PixelGraphicsPipeline> defaultGridGraphicsPipeline;
    PixelComputePipeline computePipeline;

    // images
    PixelImage emptyTexture;
    VkSampler imageSampler{};

    // Pools
    VkCommandPool graphicsCommandPool{};
    VkCommandPool computeCommandPool{};

    // validation layer component
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT debugMessenger{};

    // synchronization component
    std::vector<VkSemaphore> imageAvailableSemaphore;
    std::vector<VkSemaphore> renderFinishedSemaphore;
    std::vector<VkSemaphore> computeFinishedSemaphore;
    std::vector<VkFence> inFlightDrawFences;
    std::vector<VkFence> inFlightComputeFences;
    int currentFrame = 0;

    // objects
    std::vector<PixelScene> scenes;
    PixelScene defaultGridScene{};

    //---------vulkan functions
    // create functions
    void createInstance(VkInstance* instance);
    void setupPhysicalDevice(VkInstance* instance, VkPhysicalDevice* physicalDevice);
    void createLogicalDevice(VkDevice* device, VkPhysicalDevice* physicalDevice);
    void createSurface(VkSurfaceKHR* surface, VkInstance* instance, GLFWwindow* window);
    void createSwapChain(PixSwapchain* swapchain, PixBackend* devices, VkSurfaceKHR* surface);
    void createGraphicsPipelines();
    void createFramebuffers();
    void createCommandPools();
    void createCommandBuffers();
    void createComputeCommandBuffers();
    void createScene();
    void createDefaultGridScene();
    void initializeScenes();
    void initializeScene(PixelScene* scene);
    void createSynchronizationObjects();
    void recordCommands(uint32_t currentImageIndex);
    void recordComputeCommands(uint32_t currentImageIndex);
    VkCommandBuffer beginSingleUseCommandBuffer();
    void submitAndEndSingleUseCommandBuffer(VkCommandBuffer *commandBuffer);
    QueueFamilyIndices setupQueueFamilies(VkPhysicalDevice device);
    void init_io();
    void init_compute();
    void preDraw();

    // descriptor Set (for scene initialization)
    void createDescriptorPool(PixelScene *pixScene);
    void createDescriptorSets(PixelScene *pixScene);
    void createUniformBuffers(PixelScene *pixScene);
    void updateComputeTextureDescriptor();

    // debug validation layer
    void setupDebugMessenger(VkInstance* instance);
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    // helper functions
    bool checkIfPhysicalDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);
    void transitionImageLayout(VkImage imageToTransition, VkImageLayout currentLayout, VkImageLayout newLayout);
    void transitionImageLayoutUsingCommandBuffer(VkCommandBuffer commandBuffer, VkImage imageToTransition, VkImageLayout currentLayout,
                                                 VkImageLayout newLayout);
    void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferproperties, VkBuffer *buffer,
                      VkDeviceMemory *bufferMemory);
    void copySrcBuffertoDstBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);
    void copySrcBuffertoDstImage(VkBuffer srcBuffer, VkImage dstImageBuffer, uint32_t width, uint32_t height);

    void initializeObjectBuffers(PixelObject *pixObject);
    void createVertexBuffer(PixelObject *pixObject);
    void createIndexBuffer(PixelObject *pixObject);
    void createTextureBuffer(PixelImage *pixImage);
    void createTextureSampler();

    void updateSceneCamera(PixelScene *pixScene);

    // getter functions
    SwapchainDetails getSwapChainDetails(VkPhysicalDevice device);
};
