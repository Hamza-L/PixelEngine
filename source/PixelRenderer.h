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
	VkInstance instance{};

	//---------vulkan functions
	//create functions
	void createInstance();

	//helper functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
};

