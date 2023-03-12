#pragma once

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include "PixelWindow.h"

#include <vector>
#include <set>
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
	//window component
	PixelWindow pixWindow;

	//vulkan component
	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
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

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};	//surface properties
		std::vector<VkSurfaceFormatKHR> format;				//color and format
		std::vector<VkPresentModeKHR> presentationMode;		//how image should be presented
	};

	VkInstance instance{};
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	

	//validation layer component
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
	VkDebugUtilsMessengerEXT debugMessenger;

	//---------vulkan functions
	//create functions
	void setupDebugMessenger();
	void createInstance();
	void setupPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	QueueFamilyIndices setupQueueFamilies(VkPhysicalDevice device);

	//helper functions
	bool checkInstanceExtensionSupport(const std::vector<const char*>* checkExtensions);
	bool checkIfPhysicalDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	//getter functions
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

};

