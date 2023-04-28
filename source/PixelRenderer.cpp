#include <array>

#include "PixelRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

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
        createSwapChain();
        createCommandPool();
        createCommandBuffers();
        createScene();
        initializeScene();
        createGraphicsPipelines(); //needs the descriptor set layout of the scene
        createFramebuffers(); //need the renderbuffer from the graphics pipeline
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

    firstScene->cleanup();

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
    if(supportedDeviceFeatures.fillModeNonSolid == VK_TRUE)
    {
        deviceFeatures.fillModeNonSolid = VK_TRUE;
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
    auto graphicsPipeline = std::make_unique<PixelGraphicsPipeline>(mainDevice.logicalDevice, swapChainExtent, swapChainImageFormat);
    graphicsPipeline->addVertexShader("shaders/vert.spv");
    graphicsPipeline->addFragmentShader("shaders/frag.spv");
    graphicsPipeline->populateGraphicsPipelineInfo();
    graphicsPipeline->populatePipelineLayout(firstScene.get()); //populate the pipeline layout based on the scene's descriptor set
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

    //information on how to begin renderpass (only needed for graphical application)
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = {0,0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearValues[] = {
            {0.0f,0.0f,0.0f, 1.0f} //TODO:add depth clear color
    };
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues;

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
            //begin the renderpass
            vkCmdBeginRenderPass(commandBuffers[currentImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //our renderpass contains only primary commands

            //one pipeline can be attached per subpass. if we say we need to go to another subpass, we need to bind another pipeline.
            for(size_t p = 0; p < graphicsPipelines.size(); p++)
            {
                //bind the pipeline
                vkCmdBindPipeline(commandBuffers[currentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[p]->getPipeline());

                for(int objIndex = 0; objIndex < firstScene->getNumObjects(); objIndex++)
                {
                    VkBuffer vertexBuffers[] = {*(firstScene->getObjectAt(objIndex)->getVertexBuffer())}; //buffers to bind
                    VkBuffer indexBuffer = *firstScene->getObjectAt(objIndex)->getIndexBuffer();
                    VkDeviceSize offsets[] = {0};                                 //offsets into buffers
                    vkCmdBindVertexBuffers(commandBuffers[currentImageIndex], 0, 1, vertexBuffers, offsets);
                    vkCmdBindIndexBuffer(commandBuffers[currentImageIndex], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                    //dynamic offset ammount
                    uint32_t dynamicOffset = static_cast<uint32_t>(firstScene->getMinAlignment()) * objIndex;

                    //bind the push constant
                    vkCmdPushConstants(commandBuffers[currentImageIndex],
                                       graphicsPipelines[p]->getPipelineLayout(),
                                       VK_SHADER_STAGE_VERTEX_BIT,
                                       0,
                                       PixelObject::pushConstantRange.size,
                                       firstScene->getObjectAt(objIndex)->getPushObj());

                    //bind the descriptor sets
                    vkCmdBindDescriptorSets(commandBuffers[currentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[p]->getPipelineLayout(),
                                            0, 1, firstScene->getDescriptorSetAt(currentImageIndex), 1, &dynamicOffset);
                    //note here that we bound one descriptor set that contains both a static descriptor and a dynamic descriptor. Only the dynamic descriptors will be off-set for each object, not the static ones.

                    //execute the pipeline
                    vkCmdDrawIndexed(commandBuffers[currentImageIndex], static_cast<uint32_t>(firstScene->getObjectAt(objIndex)->getIndexCount()), 1, 0, 0, 0);
                }
            }

            //end the Renderpass
            vkCmdEndRenderPass(commandBuffers[currentImageIndex]);
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


    PixelScene::UboVP newVP{};
    newVP.P = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width/(float)swapChainExtent.height, 0.01f, 100.0f);
    newVP.V = glm::lookAt(glm::vec3(0.0f,0.0f,3.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    newVP.lightPos = glm::vec4(-2.0f,2.0f,2.0f,1.0f);


    firstScene->setSceneVP(newVP);
    firstScene->getObjectAt(0)->addTransform({glm::rotate(glm::mat4(1.0f), deltaTime,glm::vec3(1.0f,0.0f,0.0f))});
    firstScene->getObjectAt(0)->addTransform({glm::rotate(glm::mat4(1.0f), deltaTime*1.2f,glm::vec3(0.0f,1.0f,0.0f))});
    firstScene->getObjectAt(0)->addTransform({glm::rotate(glm::mat4(1.0f), deltaTime*0.4f,glm::vec3(0.0f,0.0f,1.0f))});
    firstScene->updateDynamicUniformBuffer(imageIndex);
    firstScene->updateUniformBuffer(imageIndex);

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

void PixelRenderer::createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize,
                                 VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferproperties,
                                 VkBuffer *buffer, VkDeviceMemory *bufferMemory) {

    //does not have any memory, just a header
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = bufferUsageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, buffer);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer");
        //give access to memory [beep(2) boop(9) bop(4)]

    }

    //get buffer memory requirements
    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

    //allocate memory buffer
    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits,   bufferproperties);

    result = vkAllocateMemory(device, &allocateInfo, nullptr, bufferMemory);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate device memory for object");
    }

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

void PixelRenderer::createVertexBuffer(PixelObject* pixObject) {

    //temporary buffer to stage the vertex buffer before being transfered to the GPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    //create the buffer to be transfered somewhere else
    createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, pixObject->getVertexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    //Map memory to staging buffer
    void *data; //create a pointer to a point in normal memory
    vkMapMemory(mainDevice.logicalDevice, stagingBufferMemory, 0, pixObject->getVertexBufferSize(), 0, &data); //map vertex buffer memory to that point
    memcpy(data, pixObject->getVertices()->data(), (size_t)pixObject->getVertexBufferSize()); //copy memory from vertex memory to the data pointer (ie vertex buffer memory)
    vkUnmapMemory(mainDevice.logicalDevice, stagingBufferMemory); //unmap memory

    //create buffer with transfer dst bit to mark as recipient of transfer data
    //buffer memory is only accessible in gpu memory. it is a buffer used for vertices
    createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, pixObject->getVertexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 pixObject->getVertexBuffer(), pixObject->getVertexBufferMemory());

    //graphics queues are also transfer queues & graphics command pool are also transfer command pools
    copyBuffer(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, stagingBuffer, *pixObject->getVertexBuffer(), pixObject->getVertexBufferSize());

    //cleanup transferbuffer
    vkDestroyBuffer(mainDevice.logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void PixelRenderer::createIndexBuffer(PixelObject *pixObject) {
//temporary buffer to stage the vertex buffer before being transfered to the GPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    //create the buffer to be transfered somewhere else
    createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, pixObject->getIndexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    //Map memory to staging buffer
    void *data; //create a pointer to a point in normal memory
    vkMapMemory(mainDevice.logicalDevice, stagingBufferMemory, 0, pixObject->getIndexBufferSize(), 0, &data); //map vertex buffer memory to that point
    memcpy(data, pixObject->getIndices()->data(), (size_t)pixObject->getIndexBufferSize()); //copy memory from vertex memory to the data pointer (ie vertex buffer memory)
    vkUnmapMemory(mainDevice.logicalDevice, stagingBufferMemory); //unmap memory

    //create buffer with transfer dst bit to mark as recipient of transfer data
    //buffer memory is only accessible in gpu memory. it is a buffer used for vertices
    createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, pixObject->getIndexBufferSize(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 pixObject->getIndexBuffer(), pixObject->getIndexBufferMemory());

    //graphics queues are also transfer queues & graphics command pool are also transfer command pools
    copyBuffer(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, stagingBuffer, *pixObject->getIndexBuffer(), pixObject->getIndexBufferSize());

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
        createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, PixelScene::getUniformBufferSize(),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     pixScene->getUniformBuffers(i), pixScene->getUniformBufferMemories(i));

        createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, pixScene->getDynamicUniformBufferSize(),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     pixScene->getDynamicUniformBuffers(i), pixScene->getDynamicUniformBufferMemories(i));
    }
}

void PixelRenderer::initializeScene() {

    for(int i = 0 ; i < firstScene->getNumObjects(); i++)
    {
        initializeObjectBuffers(firstScene->getObjectAt(i)); //depends on graphics command pool
    }
    firstScene->initialize(mainDevice.physicalDevice);

    createUniformBuffers(firstScene.get());
    createDescriptorPool(firstScene.get());
    createDescriptorSets(firstScene.get());
}

void PixelRenderer::createScene() {

    //create scene
    firstScene = std::make_shared<PixelScene>(PixelScene(&(mainDevice.logicalDevice)));

    PixelScene::UboVP newVP{};
    newVP.P = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width/(float)swapChainExtent.height, 0.01f, 100.0f);
    newVP.V = glm::lookAt(glm::vec3(0.0f,0.0f,3.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    newVP.lightPos = glm::vec4(1.0f,0.0f,1.0f,1.0f);

    firstScene->setSceneVP(newVP);

    //create mesh
    std::vector<PixelObject::Vertex> vertices = {
            {{-0.4f,-0.4f,0.0f,1.0f},    {0.0f,0.0f,1.0f,0.0f},{1.0f, 1.0f, 0.0f, 1.0f}}, //0
            {{0.4f,-0.4f,0.0f,1.0f},     {0.0f,0.0f,1.0f,0.0f},{0.0f, 1.0f, 1.0f, 1.0f}}, //1
            {{0.4f,0.4f,0.0f,1.0f},     {0.0f,0.0f,1.0f,0.0f},{1.0f, 0.0f, 1.0f, 1.0f}},  //2
            {{-0.4f,0.4f,0.0f,1.0f},    {0.0f,0.0f,1.0f,0.0f},{1.0f, 0.0f, 0.0f, 1.0f}}   //3
    };
    std::vector<uint32_t> indices{
            1,2,0,
            2,3,0
    };

    auto object1 = PixelObject(&(mainDevice.logicalDevice), vertices, indices);
    object1.setPushObj({glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f))});

    auto object2 = PixelObject(&mainDevice.logicalDevice, "Cube");
    object2.setPushObj({glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f)),
                                        glm::vec3(0.3f))});

    //firstScene->addObject(object1);
    firstScene->addObject(object2);

}

void PixelRenderer::createDescriptorPool(PixelScene* pixScene) {

    size_t numDescriptorSets = swapChainImages.size();

    //number of descriptors and not descriptor sets. comined, it makes the pool size
    VkDescriptorPoolSize vpPoolSize{};
    vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vpPoolSize.descriptorCount = static_cast<uint32_t>(numDescriptorSets); //one descriptor per swapchain image

    VkDescriptorPoolSize dynamicModelPoolSize{};
    dynamicModelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dynamicModelPoolSize.descriptorCount = static_cast<uint32_t>(numDescriptorSets); //one descriptor per swapchain image

    std::array<VkDescriptorPoolSize, 2> poolSizes = {vpPoolSize, dynamicModelPoolSize};

    //includes info about the descriptor set that contains the descriptor
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = static_cast<uint32_t>(numDescriptorSets); //maximum number of descriptor sets that can be created from pool
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
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(numImages, *pixScene->getDescriptorSetLayout());

    //allocate info for descriptor set. they are not created but allocated from the pool
    VkDescriptorSetAllocateInfo setAllocateInfo{};
    setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocateInfo.descriptorPool = *pixScene->getDescriptorPool();
    setAllocateInfo.descriptorSetCount = static_cast<uint32_t>(numImages);
    setAllocateInfo.pSetLayouts = descriptorSetLayouts.data();  //matches the number of swapchain images but they are all the same.
                                                                // has to be 1:1 relationship with descriptor sets

    VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocateInfo, pixScene->getDescriptorSets()->data());
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor set");
    }

    //all of the descriptor pool and descriptor set created are used to build these following struct
    for(size_t i = 0; i < pixScene->getDescriptorSets()->size() ; i++)
    {

        //BINDING 0 --------
        VkDescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo.buffer = *pixScene->getUniformBuffers(i); //buffer to get data from
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = PixelScene::getUniformBufferSize();

        VkWriteDescriptorSet vpBufferSet{};
        vpBufferSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vpBufferSet.dstSet = *pixScene->getDescriptorSetAt(i);
        vpBufferSet.dstBinding = 0; //matches layout(binding = 0)
        vpBufferSet.dstArrayElement = 0; //index in the array we want to update. we don't have an array to update here
        vpBufferSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vpBufferSet.descriptorCount = 1;
        vpBufferSet.pBufferInfo = &descriptorBufferInfo;

        //BINDING 0 --------
        VkDescriptorBufferInfo descriptorDynamicBufferInfo{};
        descriptorDynamicBufferInfo.buffer = *pixScene->getDynamicUniformBuffers(i); //buffer to get data from
        descriptorDynamicBufferInfo.offset = 0;
        descriptorDynamicBufferInfo.range = pixScene->getMinAlignment(); //what is the size of one chunk of memory

        VkWriteDescriptorSet dynamicBufferSet{};
        dynamicBufferSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        dynamicBufferSet.dstSet = *pixScene->getDescriptorSetAt(i);
        dynamicBufferSet.dstBinding = 1; //matches layout(binding = 0)
        dynamicBufferSet.dstArrayElement = 0; //index in the array we want to update. we don't have an array to update here
        dynamicBufferSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        dynamicBufferSet.descriptorCount = 1;
        dynamicBufferSet.pBufferInfo = &descriptorDynamicBufferInfo;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {vpBufferSet, dynamicBufferSet};

        //update the descriptor sets with new buffer binding info
        vkUpdateDescriptorSets(mainDevice.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);


    }

}
