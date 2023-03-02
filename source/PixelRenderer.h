#pragma once

#include "PixelWindow.h"

#include <vector>

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

	//---------vulkan functions
	//create functions
	void createInstance();
	void setupPhysicalDevice();
	QueueFamilyIndices setupQueueFamilies(VkPhysicalDevice device);

	//helper functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkIfPhysicalDeviceSuitable(VkPhysicalDevice device);
};

