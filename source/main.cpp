
#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN //includes vulkan automatically
#include <GLFW/glfw3.h>

#include "PixelRenderer.h"

int main()
{

	PixelRenderer pixRenderer;

	if (pixRenderer.initRenderer() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	

	while (!pixRenderer.windowShouldClose())
	{
		glfwPollEvents();
        pixRenderer.draw();
	}

	pixRenderer.cleanup();

	return 0;
}