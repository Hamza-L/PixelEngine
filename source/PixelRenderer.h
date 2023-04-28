#pragma once

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include "PixelWindow.h"
#include "PixelGraphicsPipeline.h"
#include "Utility.h"


#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include <memory>
#include <cstring>

const int MAX_FRAME_DRAWS = 2; //we always have "MAX_FRAME_DRAWS" being drawing at once.

class PixelRenderer
{
public:
    PixelRenderer() = default;
	PixelRenderer(const PixelRenderer&) = delete;

    PixelRenderer& operator=(const PixelRenderer&) = delete;
	~PixelRenderer() = default;

	int initRenderer();
    void draw();
	bool windowShouldClose();
	void cleanup();

    float currentTime = 0;

private:

	//window component
	PixelWindow pixWindow{};

	//vulkan component
#ifdef __APPLE__
	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_portability_subset"
	};
#else
    const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
#endif

    //logical and physical device
    PixDevice mainDevice;

    //physical device features the logical device will be using
    VkPhysicalDeviceFeatures deviceFeatures = {};

	VkInstance instance{};
	VkQueue graphicsQueue{};
	VkQueue presentationQueue{};
	VkSurfaceKHR surface{};
	VkSwapchainKHR swapChain{};
    VkRenderPass renderPass{}; //the renderer has to have atleast one renderpass to fall back on
	std::vector<PixelImage> swapChainImages;
    PixelImage depthImage;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<std::unique_ptr<PixelGraphicsPipeline>> graphicsPipelines;


	// Utility
	VkFormat swapChainImageFormat{};
	VkExtent2D swapChainExtent{};

    // Pools
    VkCommandPool graphicsCommandPool{};

	//validation layer component
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
	VkDebugUtilsMessengerEXT debugMessenger{};

    //synchronization component
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence> drawFences;
    int currentFrame = 0;

    //objects
    std::shared_ptr<PixelScene> firstScene;

	//---------vulkan functions
	//create functions
	void createInstance();
	void setupPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
    void createGraphicsPipelines();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
	void createScene();
	void initializeScene();
    void createSynchronizationObjects();
    void recordCommands(uint32_t currentImageIndex);
	QueueFamilyIndices setupQueueFamilies(VkPhysicalDevice device);

	//descriptor Set (for scene initialization)
	void createDescriptorPool(PixelScene* pixScene);
	void createDescriptorSets(PixelScene* pixScene);
	void createUniformBuffers(PixelScene* pixScene);

    //debug validation layer
    void setupDebugMessenger();
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	//helper functions
	bool checkIfPhysicalDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities); //TODO: put in utility
    static void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize,
                             VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferproperties,
                             VkBuffer* buffer, VkDeviceMemory* bufferMemory);

    void initializeObjectBuffers(PixelObject* pixObject);
    void createVertexBuffer(PixelObject* pixObject);
    void createIndexBuffer(PixelObject* pixObject);
	//getter functions
	SwapchainDetails getSwapChainDetails(VkPhysicalDevice device);

};

