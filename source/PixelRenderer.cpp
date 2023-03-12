#include "PixelRenderer.h"

//We have to look up the address of the debug callback create function ourselves using vkGetInstanceProcAddr
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//We have to look up the address of the debug callback destroy function ourselves using vkGetInstanceProcAddr
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

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
		createSurface();
		setupDebugMessenger();
		setupPhysicalDevice();
		createLogicalDevice();
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
	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(instance, nullptr);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
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
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;

	// create list to hold instance extension
	std::vector<const char*> instanceExtensions = getRequiredExtensions();	

	//check if glfw instance extensions are supported
	if (!checkInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("vkinstance does not support the required extensions\n");
	}

	//check if glfw instance extensions are supported
	if (!checkInstanceExtensionSupport(&validationLayers))
	{
		throw std::runtime_error("validation layers requested, but not available!\n");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	//for the validation layer
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.pNext = nullptr;
	}
	
	//create the vulkan instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a vulkan instance \n");
	}

}

void PixelRenderer::setupPhysicalDevice()
{
	// Enumerate the gpu devices available and fill list
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		std::runtime_error("Cannot find any GPU device with vulkan support\n");
	}

	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	for (const auto& device : deviceList)
	{
		if (checkIfPhysicalDeviceSuitable(device))
		{
			mainDevice.physicalDevice = deviceList[0];
			break;
		}
	}
	
}

void PixelRenderer::createLogicalDevice()
{
	//get the queue families for the physical device
	QueueFamilyIndices indices = setupQueueFamilies(mainDevice.physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };

	//Queue the logical device needs to create
	for (int queueFamilyIndex : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex; //index of the family to create a queue from
		queueCreateInfo.queueCount = 1;
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority; // 1 is highest, 0 is lowest. 

		queueCreateInfos.push_back(queueCreateInfo);
	}
	
	//info to create logical device (or simply device)
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); //these are logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = 0; //validation layers
	deviceCreateInfo.ppEnabledLayerNames = nullptr;

	//physical device features the logical device will be using
	VkPhysicalDeviceFeatures deviceFeatures = {};
	
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures; 

	//create logical device for the given phyisical device
	VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Error creating the logical device\n");
	}

	//we have implicitely created the graphics queue (using deviceQueueCreateInfo. we want access to them
	//from given logical device, of given queue family, of given queue index (only have 1 queue so queueIndex = 0)
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

void PixelRenderer::createSurface()
{
	//create surface (helper function creating a surface create info struct for us, returns result)
	VkResult result = glfwCreateWindowSurface(instance, pixWindow.getWindow(), nullptr, &surface);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create surface\n");
	}
}

void PixelRenderer::setupDebugMessenger()
{
	//exit function if validation layer is not enabled
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	createInfo.pUserData = nullptr; // Optional

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

PixelRenderer::QueueFamilyIndices PixelRenderer::setupQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	//get all queue family property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	//go through each q family and check if it has one of the required types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList)
	{
		//check validity of graphics q family
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i; //if queue family is valid, we keep its index
		}

		//check if queue family supports presentation
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);

		if (queueFamilyCount > 0 && presentationSupport)
		{
			indices.presentationFamily = i;
		}

		if (indices.isValid())
		{
			break;
		}

		i++;
	}

	return indices;
}

bool PixelRenderer::checkInstanceExtensionSupport(const std::vector<const char*>* checkExtensions)
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

bool PixelRenderer::checkIfPhysicalDeviceSuitable(VkPhysicalDevice device)
{
	//check the properties of the device that has been passed down (ie vendor, ID, etc..)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	//get the features supported by the GPU
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = setupQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainValid = false;
	if (extensionsSupported)
	{
	SwapChainDetails swapChainDetails = getSwapChainDetails(device);
	swapChainValid = !swapChainDetails.format.empty() && !swapChainDetails.presentationMode.empty();
	}

	return indices.isValid() && extensionsSupported && swapChainValid;
}

bool PixelRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0)
	{
		return false;
	}
	
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	for (const auto& deviceExtensions : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtensions, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			throw std::runtime_error("Extensions are not supported by the current device\n");
		}
	}

	return true;
}

std::vector<const char*> PixelRenderer::getRequiredExtensions()
{
	std::vector<const char*> extensions;

	// Set up glfw extensions instance will use
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//add glfw extensions to list of extentions
	for (size_t i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void PixelRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}

PixelRenderer::SwapChainDetails PixelRenderer::getSwapChainDetails(VkPhysicalDevice device)
{
	SwapChainDetails swapChainDetails;

	//get the surface capabilities for the surface on the given device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);

	//get the formats
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		swapChainDetails.format.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.format.data());
	}
 
	//get presentation modes
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);
	if (presentationCount != 0)
	{
		swapChainDetails.presentationMode.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationMode.data());
	}

	return swapChainDetails;
}
