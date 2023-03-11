#pragma once

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include "PixelWindow.h"

#include <vector>
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
	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;
	VkQueue graphicsQueue;

	struct QueueFamilyIndices
	{
		int graphicsFamily = -1;

		//check if queue families are valid
		bool isValid()
		{
			return graphicsFamily >= 0;
		}
	};

	VkInstance instance{};
	VkDebugUtilsMessengerEXT debugMessenger;

	//validation layer component
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	//---------vulkan functions
	//create functions
	void createInstance();
	void setupPhysicalDevice();
	void createLogicalDevice();
	void setupDebugMessenger();
	QueueFamilyIndices setupQueueFamilies(VkPhysicalDevice device);

	//helper functions
	bool checkInstanceExtensionSupport(const std::vector<const char*>* checkExtensions);
	bool checkIfPhysicalDeviceSuitable(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

};

