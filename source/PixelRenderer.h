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

class PixelRenderer
{
public:
	PixelRenderer();
	PixelRenderer(const PixelRenderer&) = delete;
	PixelRenderer& operator=(const PixelRenderer&) = delete;
	~PixelRenderer();

	int initRenderer();
	bool windowShouldClose();
	void cleanup();

private:
	
	class PixImage
	{
	public:
		VkImage image{};
		VkImageView imageView{};

		PixImage(PixelRenderer& parent);
		~PixImage();
		void cleanUp();

		//create functions
		void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
		
		//setter
		void setName(std::string name);

		//getter
		std::string getName();
		
	private:
		PixelRenderer& pixelRenderer;
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
	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	VkInstance instance{};
	VkQueue graphicsQueue{};
	VkQueue presentationQueue{};
	VkSurfaceKHR surface{};
	VkSwapchainKHR swapChain{};
	std::vector<PixImage> swapChainImages;

	// Utility
	VkFormat swapChainImageFormat{};
	VkExtent2D swapChainExtent{};

	//validation layer component
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
	VkDebugUtilsMessengerEXT debugMessenger{};

	//---------vulkan functions
	//create functions
	void setupDebugMessenger();
	void createInstance();
	void setupPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwapchain();
	QueueFamilyIndices setupQueueFamilies(VkPhysicalDevice device);

	//helper functions
	bool checkInstanceExtensionSupport(const std::vector<const char*>* checkExtensions);
	bool checkIfPhysicalDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);
	VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR surfaceCapabilities);

	//getter functions
	SwapchainDetails getSwapChainDetails(VkPhysicalDevice device);

};

