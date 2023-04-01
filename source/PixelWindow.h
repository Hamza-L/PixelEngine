#pragma once

#define GLFW_INCLUDE_VULKAN //includes vulkan automatically
#include <GLFW/glfw3.h>

#include <cstring>
#include <stdexcept>

class PixelWindow
{
public:
	PixelWindow();
	PixelWindow(const PixelWindow&) = delete;
	PixelWindow& operator=(const PixelWindow&) = delete;
	~PixelWindow();

	void initWindow(std::string wName = "Default Window", const int width = 800, const int height = 600);
	bool shouldClose();
	GLFWwindow* getWindow();

private:
	GLFWwindow* window;
	std::string windowName;
	int windowWidth;
	int windowHeight;
};

