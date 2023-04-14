#include <array>
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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {


    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
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
		createSwapchain();
        createGraphicsPipelines();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        recordCommands();
        createSynchronizationObjects();
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
    vkDeviceWaitIdle(mainDevice.logicalDevice); //wait that no action is running before destroying the objects

    for(size_t i = 0; i<MAX_FRAME_DRAWS; i++)
    {
        vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
        vkDestroySemaphore(mainDevice.logicalDevice, renderFinished[i], nullptr);
        vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
    }

    vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);

    for (auto frameBuffer : swapchainFramebuffers)
    {
        vkDestroyFramebuffer(mainDevice.logicalDevice, frameBuffer, nullptr);
    }

    for (const auto & graphicsPipeline : graphicsPipelines)
    {
        graphicsPipeline->cleanUp();
    }

	for (PixImage image : swapChainImages)
	{
		image.cleanUp();
	}

	vkDestroySwapchainKHR(mainDevice.logicalDevice, swapChain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
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
#ifdef __APPLE__
	createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
    createInfo.flags = 0;
#endif
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

void PixelRenderer::createSwapchain()
{
	//get swapchain details so we can pick best settings
	SwapchainDetails swapChainDetails = getSwapChainDetails(mainDevice.physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.format);
	VkPresentModeKHR surfacePresentationMode = chooseBestPresentationMode(swapChainDetails.presentationMode);
	VkExtent2D surfaceExtent = chooseSwapChainExtent(swapChainDetails.surfaceCapabilities);
	
	//how many images are in the swapchain. get 1 more then the minimum for triple buffering
	uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 && //if the max image count = 0, we have no max image
		swapChainDetails.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.presentMode = surfacePresentationMode;
	swapChainCreateInfo.imageExtent = surfaceExtent;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageArrayLayers = 1; 
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //don't do any blending
	swapChainCreateInfo.clipped = VK_TRUE;

	// get queue family indices
	QueueFamilyIndices indices = setupQueueFamilies(mainDevice.physicalDevice);

	//if graphics and present queues are different, they have to be shared
	if (indices.graphicsFamily != indices.presentationFamily)
	{
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentationFamily };

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 1;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr; //no need to specifiy it since there is only one
	}

	//if old swapchain been destroyed and this one replaces it, then link old swapchain to hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	swapChainCreateInfo.surface = surface;

	VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &swapChain);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create a swapChain\n");
	}

	//store for later reference
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = surfaceExtent;

	//get the vkImages from the swapChain
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, images.data());

	for (VkImage image : images)
	{
		PixImage swapChainImage = {*this};
		swapChainImage.setImage(image);
		
		swapChainImage.createImageView(swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		swapChainImages.push_back(swapChainImage);
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
	SwapchainDetails swapChainDetails = getSwapChainDetails(device);
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

#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

	return extensions;
}

void PixelRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}

//best format is subjective but ours is: 
// format: VK_FORMAT_R8G8B8A8_UNORM (backup : VK_FORMAT_B8G8R8A8_UNORM)
// colorspace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR PixelRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return formats[0];
}

VkPresentModeKHR PixelRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes)
{
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentationMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; //this is always available so if the desired present mode is not found, we return FIFO Present mode
}

VkExtent2D PixelRenderer::chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR surfaceCapabilities)
{
	// if current excent at numeric limits then extent can vary.
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else { //if value can vary, we need to set it manually
		int width, height;
		glfwGetFramebufferSize(pixWindow.getWindow(), &width, &height);

		//create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		//surface also defines max and min. we need to stay within boundary
		newExtent.width = std::max(std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width), surfaceCapabilities.maxImageExtent.width);
		newExtent.height = std::max(std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height), surfaceCapabilities.maxImageExtent.height);

		return newExtent;
	}
}

PixelRenderer::SwapchainDetails PixelRenderer::getSwapChainDetails(VkPhysicalDevice device)
{
	SwapchainDetails swapChainDetails;

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

void PixelRenderer::createGraphicsPipelines() {
    auto graphicsPipeline = std::make_unique<PixelGraphicsPipeline>(mainDevice.logicalDevice, swapChainExtent, swapChainImageFormat);
    graphicsPipeline->addVertexShader("shaders/vert.spv");
    graphicsPipeline->addFragmentShader("shaders/frag.spv");
    graphicsPipeline->populateGraphicsPipelineInfo();
    graphicsPipeline->createGraphicsPipeline(nullptr); //creates a renderpass if none were provided

    //get a default renderpass for the renderer. The renderer does not need to destroy it
    renderPass = graphicsPipeline->getRenderPass();

    graphicsPipelines.push_back(std::move(graphicsPipeline));
}

void PixelRenderer::createFramebuffers() {

    swapchainFramebuffers.resize(swapChainImages.size());

    for(size_t i =0 ; i <swapchainFramebuffers.size(); i++)
    {
        //matches the RenderBuffer Attachment
        std::array<VkImageView , 1> attachment = {
                swapChainImages[i].getImageView()
        };

        //create a framebuffer for each swapchain images;
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = graphicsPipelines[0]->getRenderPass(); //grab the first graphics pipeline's renderpass
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachment.size());
        framebufferCreateInfo.pAttachments = attachment.data();
        framebufferCreateInfo.width = swapChainExtent.width;
        framebufferCreateInfo.height = swapChainExtent.height;
        framebufferCreateInfo.layers = (uint32_t)1;

        VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void PixelRenderer::createCommandPool() {

    QueueFamilyIndices graphicsFamily = setupQueueFamilies(mainDevice.physicalDevice);

    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = graphicsFamily.graphicsFamily;

    VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolCreateInfo, nullptr, &graphicsCommandPool);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Command Pool");
    }
}

void PixelRenderer::createCommandBuffers() {

    //one commandbuffer per swapchain images
    commandBuffers.resize(swapChainImages.size());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = graphicsCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //submitted straight to the queue. Secondary cannot be called by a queue, but can only be called by other commands buffers
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()); //we create all the command buffers simultaneously
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers");
    }
    //no need to dealocate or destroyed the command buffers. they are destroy along the command pool
}

void PixelRenderer::recordCommands() {

    //info about how to begin each command buffer
    VkCommandBufferBeginInfo bufferBeginInfo{};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //flag not needed because fences are now implemented; no commandbuffer for the same frame will be submitted twice.
    //bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //if one of our command buffer is already on the queue, can it be in submitted again.

    //information on how to begin renderpass (only needed for graphical application)
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = {0,0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearValues[] = {
            {0.25f,0.25f,0.25f, 1.0f} //TODO:add depth clear color
    };
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues;

    for(size_t i = 0; i < commandBuffers.size(); i++)
    {
        renderPassBeginInfo.framebuffer = swapchainFramebuffers[i]; // the framebuffer changes per swapchain image (ie command buffer)

        VkResult result = vkBeginCommandBuffer(commandBuffers[i], &bufferBeginInfo);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to being recording command");
        }

        /*
         * Series of command to record
         * */
        {
            //begin the renderpass
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //our renderpass contains only primary commands

            //one pipeline can be attached per subpass. if we say we need to go to another subpass, we need to bind another pipeline.
            for(size_t p = 0; p < graphicsPipelines.size(); p++)
            {
                //bind the pipeline
                vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[p]->getPipeline());

                //execute the pipeline
                vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
            }

            //end the Renderpass
            vkCmdEndRenderPass(commandBuffers[i]);
        }
        /*
         * End of the series of command to record
         * */

        result = vkEndCommandBuffer(commandBuffers[i]);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to end recording command");
        }
    }

}

void PixelRenderer::draw() {

    //get the next available image to draw to and set something to signal when we are finished with the image
    //submit the command buffer to the queue for execution make sure to wait for image to be signal as available before drawing to it. it then signals when it is finished rendering
    //present image to screen when image is signaled as finished rendering

    //the only thing that will open this fence is the vkQueueSubmit
    vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame],VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);

    //Get index of the next image to draw to and signal semaphore
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mainDevice.logicalDevice,
                          swapChain,
                          std::numeric_limits<uint64_t>::max(),
                          imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailable[currentFrame]; //list of semaphores to wait on
    VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.pWaitDstStageMask = waitStages; //stage to check semaphores at
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex]; //command buffer to submit
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinished[currentFrame]; //semaphores to signal when the command buffer is finished

    //submit this commandBuffer[imageIndex] to this graphicsQueue. it's essentially our execute function
    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit queue");
    }

    //present the rendered image to the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinished[currentFrame]; //semaphore to wait for
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present image");
    }

    currentFrame = ( currentFrame + 1 ) % MAX_FRAME_DRAWS;
}

void PixelRenderer::createSynchronizationObjects() {

    imageAvailable.resize(MAX_FRAME_DRAWS);
    renderFinished.resize(MAX_FRAME_DRAWS);
    drawFences.resize(MAX_FRAME_DRAWS);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result;

    for(size_t i = 0; i<MAX_FRAME_DRAWS; i++)
    {
        result = vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailable[i]);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create the imageAvailable Semaphore");
        }

        result = vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinished[i]);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create the renderFinished Semaphore");
        }

        result = vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &drawFences[i]);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create the draw fences");
        }
    }


}

PixelRenderer::PixImage::PixImage(PixelRenderer& parent) : pixelRenderer(parent)
{
	if (parent.mainDevice.logicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Logical device not created. current Image cannot be initialized");
	}
}

PixelRenderer::PixImage::~PixImage()= default;

void PixelRenderer::PixImage::cleanUp()
{
	vkDestroyImageView(pixelRenderer.mainDevice.logicalDevice, imageView, nullptr);
	//vkDestroyImage(pixelRenderer.mainDevice.logicalDevice, image, nullptr); //if swapchain image. it will be destroyed by the swapchain
}

//create an image view for the image
void PixelRenderer::PixImage::createImageView(VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	//Subresource allow the view to view only a part of an image
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags; //which aspect of image to use (color bit for viewing color)
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0; //start mip map level to view from
	imageViewCreateInfo.subresourceRange.levelCount = 1; //levels of mip map to view
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0; //start array level to view from
	imageViewCreateInfo.subresourceRange.layerCount = 1; //layers to view

	//create image view and return it
	VkResult result = vkCreateImageView(pixelRenderer.mainDevice.logicalDevice, &imageViewCreateInfo, nullptr, &imageView);

	if (result != VK_SUCCESS)
	{
		std::string message = "Was not able to create image view for image: ";
		message.append(imageName);
		throw std::runtime_error(message.c_str());
	}
}

std::string PixelRenderer::PixImage::getName()
{
	return imageName;
}

void PixelRenderer::PixImage::setName(std::string name)
{
	imageName.clear();
	imageName = name;
}
