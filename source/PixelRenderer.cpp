#include <array>

#include "PixelRenderer.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

static int texIndex = 0;
static int itemIndex = 0;

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
	pixWindow.initWindow("PixelRenderer", 2000, 1000);
	try {
        createInstance();
		createSurface();
		setupDebugMessenger();
		setupPhysicalDevice();
		createLogicalDevice();
        createSwapChain();
        createDepthBuffer();
        createCommandPool();
        createTextureSampler();
        createCommandBuffers();
        createScene();
        initializeScenes();
        createGraphicsPipelines(); //needs the descriptor set layout of the scene
        createFramebuffers(); //need the renderbuffer for the graphics pipeline
        createSynchronizationObjects();
        init_imgui();
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

    vkDestroySampler(mainDevice.logicalDevice, imageSampler, nullptr);

    emptyTexture.cleanUp();

    for(auto & scene : scenes)
    {
        scene->cleanup();
    }

    vkDestroyDescriptorPool(mainDevice.logicalDevice, imguiPool, nullptr);
    ImGui_ImplVulkan_Shutdown();

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
    for (auto frameBuffer : swapchainFramebuffersNoDepth)
    {
        vkDestroyFramebuffer(mainDevice.logicalDevice, frameBuffer, nullptr);
    }

    for (const auto& graphicsPipeline : graphicsPipelines)
    {
        graphicsPipeline->cleanUp();
    }

    //cleaning up all swapchain images and depth image
    depthImage.cleanUp();
	for (PixelImage image : swapChainImages)
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

    if (enableValidationLayers) {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef __APPLE__
    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

	//check if glfw instance extensions are supported
	if (!checkInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("vkinstance does not support the required extensions\n");
	}

	//check if glfw instance extensions are supported
	if (!checkInstanceLayerSupport(&validationLayers))
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
		throw std::runtime_error("Cannot find any GPU device with vulkan support\n");
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

    //supported features
    VkPhysicalDeviceFeatures supportedDeviceFeatures{};
    vkGetPhysicalDeviceFeatures(mainDevice.physicalDevice, &supportedDeviceFeatures);

    //enable solid line
    if(supportedDeviceFeatures.fillModeNonSolid == VK_TRUE && supportedDeviceFeatures.samplerAnisotropy == VK_TRUE)
    {
        deviceFeatures.fillModeNonSolid = VK_TRUE; //enable fill mode nonsolid to allow for wireframe view
        deviceFeatures.samplerAnisotropy = VK_TRUE; //enable the anisotropy filtering
    }

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

void PixelRenderer::createSwapChain()
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
		PixelImage swapChainImage = {&mainDevice, swapChainExtent.width, swapChainExtent.height, true};
		swapChainImage.setImage(image);

		swapChainImage.createImageView(swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		swapChainImages.push_back(swapChainImage);
	}

    //now that we swapchain image have been
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

QueueFamilyIndices PixelRenderer::setupQueueFamilies(VkPhysicalDevice device)
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

bool PixelRenderer::checkIfPhysicalDeviceSuitable(VkPhysicalDevice device)
{
	//check the properties of the device that has been passed down (ie vendor, ID, etc..)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	//get the features supported by the GPU
	VkPhysicalDeviceFeatures supportedDeviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedDeviceFeatures);

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

	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
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

void PixelRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
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

SwapchainDetails PixelRenderer::getSwapChainDetails(VkPhysicalDevice device)
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

    //pipeline1
    auto graphicsPipeline1 = std::make_unique<PixelGraphicsPipeline>(mainDevice.logicalDevice, swapChainExtent);
    graphicsPipeline1->addVertexShader("shaders/vert.spv");
    graphicsPipeline1->addFragmentShader("shaders/frag.spv");
    graphicsPipeline1->populateGraphicsPipelineInfo();
    graphicsPipeline1->addRenderpassColorAttachment(swapChainImages[0].getFormat(),
                                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                   VK_ATTACHMENT_STORE_OP_STORE,
                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    graphicsPipeline1->addRenderpassDepthAttachment(depthImage);
    graphicsPipeline1->populatePipelineLayout(scenes[0].get()); //populate the pipeline layout based on the scene's descriptor set

    graphicsPipeline1->setScreenDimensions(0,1000,0,1000);
    graphicsPipeline1->createGraphicsPipeline(VK_NULL_HANDLE); //creates a renderpass if none were provided

    //pipeline2
    auto graphicsPipeline2 = std::make_unique<PixelGraphicsPipeline>(mainDevice.logicalDevice, swapChainExtent);
    graphicsPipeline2->addVertexShader("shaders/NoLightingShaderVert.spv");
    graphicsPipeline2->addFragmentShader("shaders/NoLightingShaderFrag.spv");
    graphicsPipeline2->populateGraphicsPipelineInfo();
    graphicsPipeline2->addRenderpassColorAttachment(swapChainImages[0].getFormat(),
                                                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                    VK_ATTACHMENT_STORE_OP_STORE,
                                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    //graphicsPipeline2->addRenderpassDepthAttachment(depthImage);
    graphicsPipeline2->populatePipelineLayout(scenes[1].get()); //populate the pipeline layout based on the scene's descriptor set

    graphicsPipeline2->setScreenDimensions(1000, 2000, 0, 1000);
    graphicsPipeline2->createGraphicsPipeline(VK_NULL_HANDLE); //creates a renderpass if none were provided

    //pipeline2
    auto graphicsPipeline3 = std::make_unique<PixelGraphicsPipeline>(mainDevice.logicalDevice, swapChainExtent);
    graphicsPipeline3->addVertexShader("shaders/NoLightingShaderVert.spv");
    graphicsPipeline3->addFragmentShader("shaders/NoLightingShaderFrag.spv");
    graphicsPipeline3->populateGraphicsPipelineInfo();
    graphicsPipeline3->addRenderpassColorAttachment(swapChainImages[0].getFormat(),
                                                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                    VK_ATTACHMENT_STORE_OP_STORE,
                                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    //graphicsPipeline3->addRenderpassDepthAttachment(depthImage);
    graphicsPipeline3->populatePipelineLayout(scenes[1].get()); //populate the pipeline layout based on the scene's descriptor set

    graphicsPipeline3->setScreenDimensions(1000, 2000, 0, 1000);
    graphicsPipeline3->setPolygonMode(VK_POLYGON_MODE_LINE);
    graphicsPipeline3->createGraphicsPipeline(VK_NULL_HANDLE); //creates a renderpass if none were provided

    graphicsPipelines.push_back(std::move(graphicsPipeline1));
    graphicsPipelines.push_back(std::move(graphicsPipeline2));
    graphicsPipelines.push_back(std::move(graphicsPipeline3));

}

void PixelRenderer::createFramebuffers() {

    swapchainFramebuffers.resize(swapChainImages.size());
    swapchainFramebuffersNoDepth.resize(swapChainImages.size());

    for(size_t i =0 ; i <swapchainFramebuffers.size(); i++)
    {
        //matches the RenderBuffer Attachment. order matters
        std::vector<VkImageView> attachments = {
                swapChainImages[i].getImageView(),
                depthImage.getImageView()
        };

        //create a framebuffer for each swapchain images;
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = graphicsPipelines[0]->getRenderPass(); //grab the first graphics pipeline's renderpass
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = swapChainExtent.width;
        framebufferCreateInfo.height = swapChainExtent.height;
        framebufferCreateInfo.layers = (uint32_t)1;

        VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer");
        }
    }

    for(size_t i =0 ; i <swapchainFramebuffersNoDepth.size(); i++)
    {
        //matches the RenderBuffer Attachment. order matters
        std::vector<VkImageView> attachment = {
                swapChainImages[i].getImageView()
        };

        //create a framebuffer for each swapchain images;
        VkFramebufferCreateInfo framebufferNoDepthCreateInfo{};
        framebufferNoDepthCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        for(auto &graphicsPipeline : graphicsPipelines)
        {
            if(!graphicsPipeline->isDepthBufferEnabled())
            {
                framebufferNoDepthCreateInfo.renderPass = graphicsPipeline->getRenderPass(); //grab the graphics pipeline's renderpass
            }
        }
        framebufferNoDepthCreateInfo.attachmentCount = static_cast<uint32_t>(attachment.size());
        framebufferNoDepthCreateInfo.pAttachments = attachment.data();
        framebufferNoDepthCreateInfo.width = swapChainExtent.width;
        framebufferNoDepthCreateInfo.height = swapChainExtent.height;
        framebufferNoDepthCreateInfo.layers = (uint32_t)1;

        VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferNoDepthCreateInfo, nullptr, &swapchainFramebuffersNoDepth[i]);
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
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //automatically forces reset when a vkBeginCmdBuffer is called
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

void PixelRenderer::recordCommands(uint32_t currentImageIndex) {

    //info about how to begin each command buffer
    VkCommandBufferBeginInfo bufferBeginInfo{};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //flag not needed because fences are now implemented; no commandbuffer for the same frame will be submitted twice.
    //bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //if one of our command buffer is already on the queue, can it be in submitted again.

    //the clear values for the renderpass attachment
    std::array<VkClearValue,2> clearValues = {};
    clearValues[0].color = {0.2f,0.2f,0.2f, 1.0f}; //colorAttachment clear value
    clearValues[1].depthStencil.depth = 1.0f; //depthAttachment clear value

    //information on how to begin renderpass (only needed for graphical application)
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderArea.offset = {0,0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();
    renderPassBeginInfo.framebuffer = swapchainFramebuffers[currentImageIndex]; // the framebuffer changes per swapchain image (ie command buffer)

        VkResult result = vkBeginCommandBuffer(commandBuffers[currentImageIndex], &bufferBeginInfo);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to being recording command");
        }

        /*
         * Series of command to record
         * */
        {
            //one pipeline can be attached per subpass. if we say we need to go to another subpass, we need to bind another pipeline.
                //there is one graphics pipeline per scene
                for(int sceneIndx = 0; sceneIndx < scenes.size(); sceneIndx++)
                {
                    renderPassBeginInfo.renderPass = graphicsPipelines[sceneIndx]->getRenderPass();

                    if (sceneIndx == 0) {
                        renderPassBeginInfo.renderArea.offset = {0, 0};
                    } else {
                        renderPassBeginInfo.renderArea.offset = {1000, 0};
                    }
                    renderPassBeginInfo.renderArea.extent.width = 1000;
                    renderPassBeginInfo.renderArea.extent.height = 1000;

                    if (!graphicsPipelines[sceneIndx]->isDepthBufferEnabled()) {
                        std::array<VkClearValue, 1> clearValue = {};
                        clearValues[0].color = {0.2f, 0.2f, 0.2f, 1.0f}; //colorAttachment clear value
                        renderPassBeginInfo.framebuffer = swapchainFramebuffersNoDepth[currentImageIndex]; // the framebuffer changes per swapchain image (ie command buffer)
                    }
                    //begin the renderpass
                    vkCmdBeginRenderPass(commandBuffers[currentImageIndex], &renderPassBeginInfo,
                                         VK_SUBPASS_CONTENTS_INLINE); //our renderpass contains only primary commands

                    for(int objIndex = 0; objIndex < scenes[sceneIndx]->getNumObjects(); objIndex++) {
                        auto currentObject = scenes[sceneIndx]->getObjectAt(objIndex);
                        if(currentObject->isHidden())
                        {
                            continue;
                        }
                        VkPipeline currentGraphicsPipeline = graphicsPipelines[currentObject->getGraphicsPipelineIndex()]->getPipeline();
                        VkPipelineLayout currentPipelineLayout = graphicsPipelines[currentObject->getGraphicsPipelineIndex()]->getPipelineLayout();

                        //bind the pipeline
                        vkCmdBindPipeline(commandBuffers[currentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          currentGraphicsPipeline);


                            VkBuffer vertexBuffers[] = {
                                    *(currentObject->getVertexBuffer())}; //buffers to bind
                            VkBuffer indexBuffer = *currentObject->getIndexBuffer();
                            VkDeviceSize offsets[] = {0};                                 //offsets into buffers
                            vkCmdBindVertexBuffers(commandBuffers[currentImageIndex], 0, 1, vertexBuffers, offsets);
                            vkCmdBindIndexBuffer(commandBuffers[currentImageIndex], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                            //dynamic offset ammount
                            uint32_t dynamicOffset = static_cast<uint32_t>(scenes[sceneIndx]->getMinAlignment()) * objIndex;

                            //bind the push constant
                            vkCmdPushConstants(commandBuffers[currentImageIndex],
                                               currentPipelineLayout,
                                               VK_SHADER_STAGE_VERTEX_BIT,
                                               0,
                                               PixelObject::pushConstantRange.size,
                                               currentObject->getPushObj());

                            std::array<VkDescriptorSet, 2> descriptorSets = {
                                    *scenes[sceneIndx]->getUniformDescriptorSetAt(currentImageIndex),
                                    *scenes[sceneIndx]->getTextureDescriptorSet()};

                            //bind the descriptor sets
                            vkCmdBindDescriptorSets(commandBuffers[currentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    currentPipelineLayout,
                                                    0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
                                                    1, &dynamicOffset);
                            //note here that we bound one descriptor set that contains both a static descriptor and a dynamic descriptor. Only the dynamic descriptors will be off-set for each object, not the static ones.

                            //execute the pipeline
                            vkCmdDrawIndexed(commandBuffers[currentImageIndex],
                                             static_cast<uint32_t>(currentObject->getIndexCount()), 1, 0, 0, 0);
                    }

                    if(sceneIndx == 0)
                    {
                        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffers[currentImageIndex]);
                    }



                    //end the Renderpass
                    vkCmdEndRenderPass(commandBuffers[currentImageIndex]);
                }
        }
        /*
         * End of the series of command to record
         * */

        result = vkEndCommandBuffer(commandBuffers[currentImageIndex]);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to end recording command");
        }

}

void PixelRenderer::draw() {

    //time measurements
    float deltaTime = (float)glfwGetTime() - currentTime;
    currentTime = (float)glfwGetTime();

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


    PixelScene::UboVP newVP1{};
    newVP1.P = glm::perspective(glm::radians(35.0f), (float)swapChainExtent.height/(float)swapChainExtent.height, 0.01f, 100.0f);
    newVP1.V = glm::lookAt(glm::vec3(0.0f,0.0f,10.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    newVP1.lightPos = glm::vec4(0.0f,5.0f,25.0f,1.0f);

    PixelScene::UboVP newVP2{};
    newVP2.P = glm::mat4(1.0f);
    newVP2.V = glm::mat4(1.0f);
    newVP2.lightPos = glm::vec4(0.0f,0.0f,5.0f,1.0f);


    scenes[0]->setSceneVP(newVP1);
    scenes[1]->setSceneVP(newVP2);
    //firstScene->getObjectAt(0)->addTransform({glm::rotate(glm::mat4(1.0f), deltaTime,glm::vec3(1.0f,0.0f,0.0f))});
    glm::mat4 objTransform = glm::mat4(1.0f);
    objTransform = glm::scale(glm::mat4(1.0f),glm::vec3(1.0f)) * objTransform;
    objTransform = glm::rotate(glm::mat4(1.0f), std::sin(currentTime*1.5f)*0.5f,glm::vec3(0.0f,1.0f,0.0f)) * objTransform;
    objTransform = glm::rotate(glm::mat4(1.0f), std::cos(currentTime*1.5f)*0.5f,glm::vec3(1.0f,0.0f,0.0f)) * objTransform;
    //objTransform = translate(glm::mat4(1.0f),glm::vec3(0.0f,sin(currentTime*1.5f)*0.3f,0.0f)) * objTransform;

    //firstScene->getObjectAt(0)->addTransform({glm::rotate(glm::mat4(1.0f), currentTime,glm::vec3(0.0f,1.0f,0.0f))});
    //firstScene->getObjectAt(0)->addTransform({glm::rotate(glm::mat4(1.0f), glm::radians(45.0f),glm::vec3(1.0f,1.0f,0.0f))});
    scenes[0]->getObjectAt(0)->setTransform({objTransform});
    scenes[0]->getObjectAt(0)->setTexID(texIndex);
    scenes[0]->getObjectAt(1)->setTransform({objTransform});
    scenes[0]->getObjectAt(1)->setTexID(texIndex);
    scenes[0]->updateDynamicUniformBuffer(imageIndex);
    scenes[0]->updateUniformBuffer(imageIndex);

    scenes[1]->getObjectAt(0)->setTransform({glm::mat4(1.0f)});
    scenes[1]->getObjectAt(0)->setTexID(texIndex);
    scenes[1]->updateDynamicUniformBuffer(imageIndex);
    scenes[1]->updateUniformBuffer(imageIndex);


    switch(itemIndex) {
        case 0:
            scenes[0]->getObjectAt(0)->unhide();
            scenes[0]->getObjectAt(1)->hide();
            scenes[1]->getObjectAt(1)->unhide();
            scenes[1]->getObjectAt(2)->unhide();
            scenes[1]->getObjectAt(3)->hide();
            scenes[1]->getObjectAt(4)->hide();
            break;
        case 1:
            scenes[0]->getObjectAt(0)->hide();
            scenes[0]->getObjectAt(1)->unhide();
            scenes[1]->getObjectAt(1)->hide();
            scenes[1]->getObjectAt(2)->hide();
            scenes[1]->getObjectAt(3)->unhide();
            scenes[1]->getObjectAt(4)->unhide();
            break;
    }


    //we do not want to update all command buffers. only update the current command buffer being written to.
    recordCommands(imageIndex);

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

void PixelRenderer::run() {

    while (!glfwWindowShouldClose(pixWindow.getWindow()))
    {
        glfwPollEvents();

        //imgui new frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        imGuiParametersSetup();
        ImGui::Render();

        draw_data = ImGui::GetDrawData();

        draw();
    }
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

void PixelRenderer::createBuffer(VkDeviceSize bufferSize,
                                 VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferproperties,
                                 VkBuffer *buffer, VkDeviceMemory *bufferMemory) {

    //does not have any memory, just a header
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = bufferUsageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(mainDevice.logicalDevice, &bufferInfo, nullptr, buffer);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer");
        //give access to memory [beep(2) boop(9) bop(4)]

    }

    //get buffer memory requirements
    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(mainDevice.logicalDevice, *buffer, &memoryRequirements);

    //allocate memory buffer
    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryTypeIndex(mainDevice.physicalDevice, memoryRequirements.memoryTypeBits,   bufferproperties);

    result = vkAllocateMemory(mainDevice.logicalDevice, &allocateInfo, nullptr, bufferMemory);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate device memory for object");
    }

    vkBindBufferMemory(mainDevice.logicalDevice, *buffer, *bufferMemory, 0);
}

void PixelRenderer::copySrcBuffertoDstBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize) {

    //Command Buffer to hold the commands
    VkCommandBuffer transferCommandBuffer = beginSingleUseCommandBuffer();

    //region of data to copy from and too
    VkBufferCopy bufferCopy{};
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = 0;
    bufferCopy.size = bufferSize;

    vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

    submitAndEndSingleUseCommandBuffer(&transferCommandBuffer);

}

void PixelRenderer::createVertexBuffer(PixelObject* pixObject) {

    //temporary buffer to stage the vertex buffer before being transfered to the GPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    //create the buffer to be transfered somewhere else
    createBuffer(pixObject->getVertexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    //Map memory to staging buffer
    void *data; //create a pointer to a point in normal memory
    vkMapMemory(mainDevice.logicalDevice, stagingBufferMemory, 0, pixObject->getVertexBufferSize(), 0, &data); //map vertex buffer memory to that point
    memcpy(data, pixObject->getVertices()->data(), (size_t)pixObject->getVertexBufferSize()); //copy memory from vertex memory to the data pointer (ie vertex buffer memory)
    vkUnmapMemory(mainDevice.logicalDevice, stagingBufferMemory); //unmap memory

    //create buffer with transfer dst bit to mark as recipient of transfer data
    //buffer memory is only accessible in gpu memory. it is a buffer used for vertices
    createBuffer(pixObject->getVertexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 pixObject->getVertexBuffer(), pixObject->getVertexBufferMemory());

    //graphics queues are also transfer queues & graphics command pool are also transfer command pools
    copySrcBuffertoDstBuffer(stagingBuffer, *pixObject->getVertexBuffer(), pixObject->getVertexBufferSize());

    //cleanup transferbuffer
    vkDestroyBuffer(mainDevice.logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void PixelRenderer::createTextureBuffer(PixelImage* pixImage) {

    //temporary buffer to stage the vertex buffer before being transfered to the GPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    //create the buffer to be transfered somewhere else
    createBuffer(pixImage->getImageBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    if(pixImage->getImageData() != nullptr)
    {
        //Map memory to staging buffer
        void *data; //create a pointer to a point in normal memory
        vkMapMemory(mainDevice.logicalDevice, stagingBufferMemory, 0, pixImage->getImageBufferSize(), 0, &data); //map vertex buffer memory to that point
        memcpy(data, pixImage->getImageData(), static_cast<size_t>(pixImage->getImageBufferSize())); //copy memory from vertex memory to the data pointer (ie vertex buffer memory)
        vkUnmapMemory(mainDevice.logicalDevice, stagingBufferMemory); //unmap memory

        //transition the image to image layout transfer bit so it can receive the buffer data during the transfer stage.
        transitionImageLayout(pixImage->getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        //graphics queues are also transfer queues & graphics command pool are also transfer command pools
        copySrcBuffertoDstImage(stagingBuffer, pixImage->getImage(), pixImage->getWidth(), pixImage->getHeight());

        //transition the image from image layout transfer bit so it can be read by the shader.
        transitionImageLayout(pixImage->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    } else
    {
        transitionImageLayout(pixImage->getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }


    //cleanup transferbuffer
    vkDestroyBuffer(mainDevice.logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void PixelRenderer::createIndexBuffer(PixelObject *pixObject)
{
    //temporary buffer to stage the vertex buffer before being transfered to the GPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    //create the buffer to be transfered somewhere else
    createBuffer(pixObject->getIndexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    //Map memory to staging buffer
    void *data; //create a pointer to a point in normal memory
    vkMapMemory(mainDevice.logicalDevice, stagingBufferMemory, 0, pixObject->getIndexBufferSize(), 0, &data); //map vertex buffer memory to that point
    memcpy(data, pixObject->getIndices()->data(), (size_t)pixObject->getIndexBufferSize()); //copy memory from vertex memory to the data pointer (ie vertex buffer memory)
    vkUnmapMemory(mainDevice.logicalDevice, stagingBufferMemory); //unmap memory

    //create buffer with transfer dst bit to mark as recipient of transfer data
    //buffer memory is only accessible in gpu memory. it is a buffer used for vertices
    createBuffer(pixObject->getIndexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 pixObject->getIndexBuffer(), pixObject->getIndexBufferMemory());

    //graphics queues are also transfer queues & graphics command pool are also transfer command pools
    copySrcBuffertoDstBuffer(stagingBuffer, *pixObject->getIndexBuffer(), pixObject->getIndexBufferSize());

    //cleanup transferbuffer
    vkDestroyBuffer(mainDevice.logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void PixelRenderer::initializeObjectBuffers(PixelObject *pixObject) {
    createVertexBuffer(pixObject);
    createIndexBuffer(pixObject);
}

void PixelRenderer::createUniformBuffers(PixelScene *pixScene) {

    pixScene->resizeBuffers(swapChainImages.size());

    for(int i = 0; i < swapChainImages.size(); i++)
    {//create the buffer to be transfered somewhere else
        createBuffer(PixelScene::getUniformBufferSize(),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     pixScene->getUniformBuffers(i), pixScene->getUniformBufferMemories(i));

        createBuffer(pixScene->getDynamicUniformBufferSize(),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     pixScene->getDynamicUniformBuffers(i), pixScene->getDynamicUniformBufferMemories(i));
    }
}

void PixelRenderer::initializeScenes() {

    //load an empty texture for use when texture is not defined.
    emptyTexture = PixelImage(&mainDevice, 0, 0, false);
    emptyTexture.loadEmptyTexture();
    createTextureBuffer(&emptyTexture);

    //initialize all objects in the scene
    for(auto& scene : scenes)
    {
        for(int i = 0 ; i < scene->getNumObjects(); i++)
        {
            initializeObjectBuffers(scene->getObjectAt(i)); //depends on graphics command pool
            for(auto texture : scene->getObjectAt(i)->getTextures())
            {
                createTextureBuffer(&texture);
            }
        }

        createUniformBuffers(scene.get());
        createDescriptorPool(scene.get());
        createDescriptorSets(scene.get());
    }


}

void PixelRenderer::createScene() {

    //create scene
    std::shared_ptr<PixelScene> scene1 = std::make_shared<PixelScene>(PixelScene(mainDevice.logicalDevice, mainDevice.physicalDevice));
    std::shared_ptr<PixelScene> scene2 = std::make_shared<PixelScene>(PixelScene(mainDevice.logicalDevice, mainDevice.physicalDevice));

    PixelScene::UboVP newVP{};
    newVP.P = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width/(float)swapChainExtent.height, 0.01f, 100.0f);
    newVP.V = glm::lookAt(glm::vec3(0.0f,0.0f,3.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    newVP.lightPos = glm::vec4(0.0f,0.0f,1.0f,1.0f);

    scene1->setSceneVP(newVP);
    scene2->setSceneVP(newVP);

    //create mesh
    std::vector<PixelObject::Vertex> vertices = {
            {{-0.9f,-0.9f,0.0f,1.0f},    {0.0f,0.0f,1.0f,0.0f},{1.0f, 1.0f, 0.0f, 1.0f},{0.0f, 1.0f}}, //0
            {{0.9f,-0.9f,0.0f,1.0f},     {0.0f,0.0f,1.0f,0.0f},{0.0f, 1.0f, 1.0f, 1.0f},{1.0f, 1.0f}}, //1
            {{0.9f,0.9f,0.0f,1.0f},     {0.0f,0.0f,1.0f,0.0f},{1.0f, 0.0f, 1.0f, 1.0f},{1.0f, 0.0f}},  //2
            {{-0.9f,0.9f,0.0f,1.0f},    {0.0f,0.0f,1.0f,0.0f},{1.0f, 0.0f, 0.0f, 1.0f},{0.0f, 0.0f}}    //3
    };
    std::vector<uint32_t> indices{
            1,2,0,
            2,3,0
    };

    auto texturedMesh = PixelObject(&mainDevice, vertices, indices);
    auto mug = PixelObject(&mainDevice, "Mug.obj");
    auto skull = PixelObject(&mainDevice, "Skull.obj");

    //object1.addTexture("brick.jpg");
    mug.addTexture("MugTexture1.png");
    mug.addTexture("MugTexture2.png");
    mug.addTexture("MugTexture3.png");
    mug.addTexture("MugTexture4.png");
    skull.addTexture("Skull.jpg");
    texturedMesh.addTexture("MugTexture1.png");
    texturedMesh.addTexture("MugTexture2.png");
    texturedMesh.addTexture("MugTexture3.png");
    texturedMesh.addTexture("MugTexture4.png");
    texturedMesh.addTexture("Skull.jpg");

    //create a mesh form the texUV data
    std::vector<PixelObject::Vertex> texMeshVertices1;
    std::vector<uint32_t> texMeshIndices1;
    for(int i = 0 ; i < mug.getIndexCount(); i++)
    {
        uint32_t indx = (*mug.getIndices())[i];
        texMeshIndices1.push_back(indx); // add the same index number to the new object
    }

    for(int i = 0 ; i < mug.getVertexCount(); i++)
    {
        float texU = 0.9f * (2.0f * (*mug.getVertices())[i].texUV[0] - 1.0f);
        float texV = 0.9f * (2.0f * (*mug.getVertices())[i].texUV[1] - 1.0f);
        texMeshVertices1.push_back({
                                           {texU, -texV, 0.0f, 1.0f},
                                           {0.0f, 0.0f, 1.0f, 0.0f},
                                           {0.2f, 0.2f, 0.2f, 0.5f},
                                           {0.0f, 0.0f}
                                   });
    }

    auto mugFlat = PixelObject(&mainDevice, texMeshVertices1, texMeshIndices1);
    auto mugFlatLines = PixelObject(&mainDevice, texMeshVertices1, texMeshIndices1);

    //create a mesh form the texUV data
    std::vector<PixelObject::Vertex> texMeshVertices2;
    std::vector<uint32_t> texMeshIndices2;
    for(int i = 0 ; i < skull.getIndexCount(); i++)
    {
        uint32_t indx = (*skull.getIndices())[i];
        texMeshIndices2.push_back(indx); // add the same index number to the new object
    }

    for(int i = 0 ; i < skull.getVertexCount(); i++)
    {
        float texU = 0.9f * (2.0f * (*skull.getVertices())[i].texUV[0] - 1.0f);
        float texV = 0.9f * (2.0f * (*skull.getVertices())[i].texUV[1] - 1.0f);
        texMeshVertices2.push_back({
                                           {texU, -texV, 0.0f, 1.0f},
                                           {0.0f, 0.0f, 1.0f, 0.0f},
                                           {0.2f, 0.2f, 0.2f, 0.5f},
                                           {0.0f, 0.0f}
                                   });
    }

    auto skullFlat = PixelObject(&mainDevice, texMeshVertices2, texMeshIndices2);
    auto skullFlatLines = PixelObject(&mainDevice, texMeshVertices2, texMeshIndices2);

    mug.setGraphicsPipelineIndex(0);
    skull.setGraphicsPipelineIndex(0);
    texturedMesh.setGraphicsPipelineIndex(1);
    mugFlat.setGraphicsPipelineIndex(1);
    mugFlatLines.setGraphicsPipelineIndex(2);
    skullFlat.setGraphicsPipelineIndex(1);
    skullFlatLines.setGraphicsPipelineIndex(2);

    //firstScene->addObject(object1);
    scene1->addObject(mug);
    scene1->addObject(skull);
    scene2->addObject(texturedMesh);
    scene2->addObject(mugFlat);
    scene2->addObject(mugFlatLines);
    scene2->addObject(skullFlat);
    scene2->addObject(skullFlatLines);

    mug.setTexID(1); //TODO:problem there. value not copied

    scenes.push_back(scene1);
    scenes.push_back(scene2);

}

void PixelRenderer::createDescriptorPool(PixelScene* pixScene) {

    size_t numTextureDescriptorSet = 1;
    size_t numUniformDescriptorSets = swapChainImages.size();

    //number of descriptors and not descriptor sets. combined, it makes the pool size
    VkDescriptorPoolSize vpPoolSize{};
    vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vpPoolSize.descriptorCount = static_cast<uint32_t>(numUniformDescriptorSets); //one descriptor per swapchain image

    VkDescriptorPoolSize dynamicModelPoolSize{};
    dynamicModelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dynamicModelPoolSize.descriptorCount = static_cast<uint32_t>(numUniformDescriptorSets); //one descriptor per swapchain image

    VkDescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_OBJECTS * MAX_TEXTURE_PER_OBJECT); //one descriptor per swapchain image

    std::array<VkDescriptorPoolSize, 3> poolSizes = {vpPoolSize, dynamicModelPoolSize, samplerPoolSize};

    //includes info about the descriptor set that contains the descriptor
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = static_cast<uint32_t>(numUniformDescriptorSets + numTextureDescriptorSet); //maximum number of descriptor sets that can be created from pool
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes = poolSizes.data();

    VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &poolCreateInfo, nullptr, pixScene->getDescriptorPool());
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool");
    }

}

void PixelRenderer::createDescriptorSets(PixelScene *pixScene)
{
    //we have 1 Descriptor Set and 2 bindings. one binding for the VP matrices. one binding for the dynamic buffer object for M matrix.
    const size_t numImages = swapChainImages.size();
    //resize the descriptor sets to match the uniform buffers that contain its data
    pixScene->resizeDesciptorSets(numImages);

    //descriptor set layouts
    std::vector<VkDescriptorSetLayout> uniformDescriptorSetLayouts(numImages, *pixScene->getDescriptorSetLayout(UBOS));

    //allocate info for ubo descriptor set. they are not created but allocated from the pool
    VkDescriptorSetAllocateInfo uboSetAllocateInfo{};
    uboSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    uboSetAllocateInfo.descriptorPool = *pixScene->getDescriptorPool();
    uboSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(numImages);
    uboSetAllocateInfo.pSetLayouts = uniformDescriptorSetLayouts.data();  //matches the number of swapchain images but they are all the same.
                                                                // has to be 1:1 relationship with descriptor sets

    VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &uboSetAllocateInfo,
                                               pixScene->getUniformDescriptorSets()->data());
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor set for ubos");
    }

    //allocate info for texture descriptor set. they are not created but allocated from the pool
    VkDescriptorSetAllocateInfo textureSetAllocateInfo{};
    textureSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    textureSetAllocateInfo.descriptorPool = *pixScene->getDescriptorPool();
    textureSetAllocateInfo.descriptorSetCount = 1;
    textureSetAllocateInfo.pSetLayouts = pixScene->getDescriptorSetLayout(TEXTURES);  //matches the number of swapchain images but they are all the same.
    // has to be 1:1 relationship with descriptor sets

    result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &textureSetAllocateInfo, pixScene->getTextureDescriptorSet());
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor set for textures");
    }

    //all of the descriptor pool and descriptor set created are used to build these following struct
    for(size_t i = 0; i < pixScene->getUniformDescriptorSets()->size() ; i++)
    {

        //BINDING 0 of SET 0--------
        VkDescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo.buffer = *pixScene->getUniformBuffers(i); //buffer to get data from
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = PixelScene::getUniformBufferSize();

        VkWriteDescriptorSet vpBufferSet{};
        vpBufferSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vpBufferSet.dstSet = *pixScene->getUniformDescriptorSetAt(i);
        vpBufferSet.dstBinding = 0; //matches layout(binding = 0)
        vpBufferSet.dstArrayElement = 0; //index in the array we want to update. we don't have an array to update here
        vpBufferSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vpBufferSet.descriptorCount = 1;
        vpBufferSet.pBufferInfo = &descriptorBufferInfo;

        //BINDING 1 of SET 0 --------
        VkDescriptorBufferInfo descriptorDynamicBufferInfo{};
        descriptorDynamicBufferInfo.buffer = *pixScene->getDynamicUniformBuffers(i); //buffer to get data from
        descriptorDynamicBufferInfo.offset = 0;
        descriptorDynamicBufferInfo.range = pixScene->getMinAlignment(); //what is the size of one chunk of memory

        VkWriteDescriptorSet dynamicBufferSet{};
        dynamicBufferSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        dynamicBufferSet.dstSet = *pixScene->getUniformDescriptorSetAt(i);
        dynamicBufferSet.dstBinding = 1; //matches layout(binding = 0)
        dynamicBufferSet.dstArrayElement = 0; //index in the array we want to update. we don't have an array to update here
        dynamicBufferSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        dynamicBufferSet.descriptorCount = 1;
        dynamicBufferSet.pBufferInfo = &descriptorDynamicBufferInfo;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {vpBufferSet, dynamicBufferSet};

        //update the descriptor sets with new buffer binding info
        vkUpdateDescriptorSets(mainDevice.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    //BINDING 0 of SET 1 --------
    std::array<VkDescriptorImageInfo, MAX_TEXTURE_PER_OBJECT> textureSamplerDescriptorInfos{};
    //VkDescriptorImageInfo textureSamplerDescriptorInfo{};
    for(int i = 0 ; i < textureSamplerDescriptorInfos.size(); i++)
    {
        if(i < pixScene->getAllTextures().size())
        {
            if(pixScene->getAllTextures()[i].hasBeenInitialized()) //TODO:make sure we go through all the scenes
            {
                textureSamplerDescriptorInfos[i].imageView = pixScene->getAllTextures()[i].getImageView(); //image view of the texture
            }
            else
            {
                textureSamplerDescriptorInfos[i].imageView = emptyTexture.getImageView(); //image view of the texture
            }
        }else
        {
            textureSamplerDescriptorInfos[i].imageView = emptyTexture.getImageView(); //image view of the texture
        }
        textureSamplerDescriptorInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //what is the image layout when in use
        textureSamplerDescriptorInfos[i].sampler = imageSampler; //the image sampler
    }


    VkWriteDescriptorSet textureSamplerDescriptorSet{};
    textureSamplerDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    textureSamplerDescriptorSet.dstSet = *pixScene->getTextureDescriptorSet();
    textureSamplerDescriptorSet.dstBinding = 0; //matches layout(binding = 0)
    textureSamplerDescriptorSet.dstArrayElement = 0; //index in the array we want to update. we don't have an array to update here
    textureSamplerDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureSamplerDescriptorSet.descriptorCount = static_cast<uint32_t>(textureSamplerDescriptorInfos.size()); //number of textures
    textureSamplerDescriptorSet.pImageInfo = textureSamplerDescriptorInfos.data();

    //update the descriptor sets with new buffer binding info
    vkUpdateDescriptorSets(mainDevice.logicalDevice, 1, &textureSamplerDescriptorSet, 0, nullptr);

}

void PixelRenderer::createDepthBuffer() {

    //create our depth buffer image.
    depthImage = PixelImage(&mainDevice, swapChainExtent.width, swapChainExtent.height, false);
    depthImage.createDepthBufferImage();
}

VkCommandBuffer PixelRenderer::beginSingleUseCommandBuffer() {
    //Command Buffer to hold the commands
    VkCommandBuffer transferCommandBuffer;

    //info for transfer commandbuffer creation
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = graphicsCommandPool; //graphics command pool can act as a transfer command pool
    commandBufferAllocateInfo.commandBufferCount = 1;

    //allocate commandbuffer from pool
    vkAllocateCommandBuffers(mainDevice.logicalDevice, &commandBufferAllocateInfo, &transferCommandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //we are using it only once. we submit it and we destroy it.

    vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

    return transferCommandBuffer;
}

void PixelRenderer::submitAndEndSingleUseCommandBuffer(VkCommandBuffer* commandBuffer) {

    //end the given command buffer
    vkEndCommandBuffer(*commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer;

    //submit the transfer queue (the graphics queue is the transfer queue)
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue); //submits the queue and wait for it to stop running

    vkFreeCommandBuffers(mainDevice.logicalDevice, graphicsCommandPool, 1, commandBuffer);
}

void PixelRenderer::copySrcBuffertoDstImage(VkBuffer srcBuffer, VkImage dstImageBuffer, uint32_t width, uint32_t height) {

    //copying buffer memory to image memory
    VkCommandBuffer transferCommandBuffer = beginSingleUseCommandBuffer();

    //region of data to copy from and too
    VkBufferImageCopy bufferCopy{};
    bufferCopy.bufferOffset = 0;
    bufferCopy.bufferRowLength = 0; //for data spacing
    bufferCopy.bufferImageHeight = 0; //for data spacing. because everything is 0, the data is tightly packed
    bufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopy.imageSubresource.layerCount = 1;
    bufferCopy.imageSubresource.baseArrayLayer = 0;
    bufferCopy.imageSubresource.mipLevel = 0; //TODO:: implement mipmap level for textures
    bufferCopy.imageOffset = {0,0,0}; //start at the origin. no offset
    bufferCopy.imageExtent = {width, height, 1}; //size of the region to copy

    vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, dstImageBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &bufferCopy); //we have to specify as dst_optimal because it is receiving the data from a staging buffer.

    submitAndEndSingleUseCommandBuffer(&transferCommandBuffer);
}

void
PixelRenderer::transitionImageLayout(VkImage imageToTransition, VkImageLayout currentLayout, VkImageLayout newLayout)
{

    VkCommandBuffer commandBuffer = beginSingleUseCommandBuffer();

    // 1- allows us to specify stage dependencies
    // 2- allows us to transition image layout
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = currentLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //don't bother transfer from queue. has to be specified
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //don't bother transfer to queue. has to be specified
    imageMemoryBarrier.image = imageToTransition;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0; //TODO:: implement mipmap level for textures

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if(currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = 0; //from the very start. there is no specified stage.
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; //we want the transfer to happen before this stage
        //transfer write bit are operations like transfering memory from staging buffer to image buffer.

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if(currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; //from the very start. there is no specified stage.
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; //we want the transfer to happen before this stage
        //transfer write bit are operations like transfering memory from staging buffer to image buffer.

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }else if(currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = 0; //from the very start. there is no specified stage.
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; //we want the transfer to happen before this stage
        //transfer write bit are operations like transfering memory from staging buffer to image buffer.

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer,
                         srcStage, dstStage, //match to src and dst access mask
                         0,
                         0, nullptr, //general memory barrier
                         0, nullptr, //buffer memory marrier
                         1, &imageMemoryBarrier); //image memory barrier

    submitAndEndSingleUseCommandBuffer(&commandBuffer);
}

void PixelRenderer::createTextureSampler() {

    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR; //how to render when texture CLOSER to screen
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR; //how to render when texture FURTHER to screen
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //not used because we use repeat
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; //TODO:Implement Mip Map
    samplerCreateInfo.mipLodBias = 0.0f; //adding an offset to the mip map level
    samplerCreateInfo.minLod = 0.0f; //TODO:Implement Mip Map
    samplerCreateInfo.maxLod = 0.0f; //TODO:Implement Mip Map
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16; //number of samples taken for the anisotropy filtering

    VkResult result = vkCreateSampler(mainDevice.logicalDevice, &samplerCreateInfo, nullptr, &imageSampler);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create sampler");
    }

}

void PixelRenderer::init_imgui() {

        IMGUI_CHECKVERSION();

        // 2: initialize imgui library

        //this initializes the core structures of imgui
        ImGui::CreateContext();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        //1: create descriptor pool for IMGUI
        // the size of the pool is very oversize, but it's copied from imgui demo itself.
        VkDescriptorPoolSize pool_sizes[] =
                {
                        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &pool_info, nullptr, &imguiPool);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor pool for ImGui");
        }

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        //this initializes imgui for SDL
        ImGui_ImplGlfw_InitForVulkan(pixWindow.getWindow(), true);

        //this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = mainDevice.physicalDevice;
        init_info.Device = mainDevice.logicalDevice;
        init_info.Queue = graphicsQueue;
        init_info.QueueFamily = setupQueueFamilies(mainDevice.physicalDevice).graphicsFamily;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info, graphicsPipelines[0]->getRenderPass());

        //execute a gpu command to upload imgui font textures
        VkCommandBuffer commandBuffer = beginSingleUseCommandBuffer();
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        submitAndEndSingleUseCommandBuffer(&commandBuffer);

        //clear font textures from cpu data
        ImGui_ImplVulkan_DestroyFontUploadObjects();

}

void PixelRenderer::imGuiParametersSetup() {
    ImGui::Begin("Simple Render Engine!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::SetWindowSize(ImVec2(350.0f,200.0f),0);
    //ImGui::Text("Fog Effect intensity.");               // Display some text (you can use a format strings too)
    //static float test = 0.0f;
    //ImGui::SliderFloat("fog_pow", &test, 0.0f, 2.0f);
    ImGui::Text("import object");

    static const char* current_item = NULL;
    static const char* current_texture = NULL;
    static std::vector<std::string> textures = {
            "MugTexture1.png",
            "MugTexture2.png",
            "MugTexture3.png",
            "MugTexture4.png",
            "Skull.jpeg"
    };

    static std::vector<std::string> items = {
            "Mug.obj",
            "Skull.obj"
    };

    current_item = items[itemIndex].c_str();
    if (ImGui::BeginCombo("##combo1", current_item)) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < items.size(); n++)
        {
            bool is_selected = (current_item == items[n].c_str()); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(items[n].c_str(), is_selected)){
                current_item = items[n].c_str();
                itemIndex = n;
            }
            if (is_selected){
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("import texture");
    current_texture = textures[texIndex].c_str();
    if (ImGui::BeginCombo("##combo2", current_texture)) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < textures.size(); n++)
        {
            bool is_selected = (current_texture == textures[n].c_str()); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(textures[n].c_str(), is_selected)){
                current_texture = textures[n].c_str();
                texIndex = n;
            }
            if (is_selected){
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
        }
        ImGui::EndCombo();
    }

    //ImGui::Text("Fog Effect shift offset.");
    //ImGui::SliderFloat("fog_shift", &slider.fog_shift, 0.0f, 2.0f);


    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void PixelRenderer::addScene(PixelScene *pixScene) {

    scenes.push_back(std::make_shared<PixelScene>(*pixScene));
}
