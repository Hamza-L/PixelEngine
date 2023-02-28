#include "PixelRenderer.h"

PixelRenderer::PixelRenderer()
{
}

PixelRenderer::~PixelRenderer()
{
}

int PixelRenderer::initRenderer()
{
	pixWindow.initWindow();
	try {
		createInstance();
	}
	catch(const std::runtime_error &e)
	{
		fprintf(stderr,"ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

bool PixelRenderer::windowShouldClose()
{
	return pixWindow.shouldClose();
}

void PixelRenderer::cleanup()
{
	vkDestroyInstance(instance, nullptr);
}

void PixelRenderer::createInstance()
{
	//information about the application itself
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App";
	appInfo.pEngineName = "Pixel App";
	appInfo.apiVersion = VK_API_VERSION_1_2;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	//creation info for a vulkan instance.
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

	// create list to hold instance extension
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	// Set up glfw extensions instance will use
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//add glfw extensions to list of extentions
	for (size_t i = 0; i < glfwExtensionCount; i++)
	{
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	//check if instance extensions are supported
	if (!checkInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("vkinstance does not support the required extensions\n");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	//for the validation layer
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	//create the vulkan instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a vulkan instance \n");
	}

}

bool PixelRenderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	//get the number of extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	//create a vector of the size of extensions and populate it
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for (const auto &checkExtension: *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}

	}

	return true;

}
