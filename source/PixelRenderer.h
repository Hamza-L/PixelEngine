#pragma once

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include "PixelWindow.h"
#include "PixelGraphicsPipeline.h"

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

private:
	
	class PixImage
	{
	public:
        PixImage(PixelRenderer& parent);
		~PixImage();
		void cleanUp();

		//create functions
		void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
		
		//setter
		void setName(std::string name);
        void setImage(VkImage inputImage){image = inputImage;}
        void setImage(VkImageView inputImageView){imageView = inputImageView;}

		//getter
		std::string getName();
        VkImage getImage() { return image;}
        VkImageView getImageView() {return imageView;}
		
	private:
		PixelRenderer& pixelRenderer;

        VkImage image{};
        VkImageView imageView{};
		std::string imageName{};
	};

	//vulkan struct component
	struct PixDevices{
		VkPhysicalDevice physicalDevice{};
		VkDevice logicalDevice{};
	} mainDevice;

	struct QueueFamilyIndices
	{
		int graphicsFamily = -1;
		int presentationFamily = -1;

		//check if queue families are valid
		bool isValid()
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

	VkInstance instance{};
	VkQueue graphicsQueue{};
	VkQueue presentationQueue{};
	VkSurfaceKHR surface{};
	VkSwapchainKHR swapChain{};
    VkRenderPass renderPass{}; //the renderer has to have atleast one renderpass to fall back on
	std::vector<PixImage> swapChainImages;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<std::unique_ptr<PixelGraphicsPipeline>> graphicsPipelines;


	// Utility
	VkFormat swapChainImageFormat{};
	VkExtent2D swapChainExtent{};

    // Pools
    VkCommandPool graphicsCommandPool;

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

	//---------vulkan functions
	//create functions
	void setupDebugMessenger();
	void createInstance();
	void setupPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwapchain();
    void createGraphicsPipelines();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSynchronizationObjects();
    void recordCommands();
	QueueFamilyIndices setupQueueFamilies(VkPhysicalDevice device);

	//helper functions
	bool checkInstanceExtensionSupport(const std::vector<const char*>* checkExtensions);
	bool checkIfPhysicalDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
	VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);

	//getter functions
	SwapchainDetails getSwapChainDetails(VkPhysicalDevice device);

};

