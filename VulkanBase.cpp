#include "VulkanBase.h"
#include "vulkanHelper/VulkanHelper.h"
#include "vulkanHelper/VulkanStructs.h"


#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <set>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <thread>
#include <future>
#include <functional>

#include "GlobalObjects.h"
#include "GraphicsObjects.h"
#include "Models.h"
#include "ModelLoader.h"
#include "UI.h"
#include "Utilities.h"
using namespace Utility;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION    
#include <stb/stb_image.h>

#define SAMPLE_NAME "Sample name"
// old
std::vector<const char*> VulkanBase::getRequiredExtensions() const
{
	uint32_t glfwEextensionsCount;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwEextensionsCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwEextensionsCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool VulkanBase::checkValidationLayerSupport() const
{
	uint32_t availableLayerCount;
	vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(availableLayerCount);
	vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const VkLayerProperties& availableLayer : availableLayers)
		{
			if (strcmp(layerName, availableLayer.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<char> VulkanBase::readFile(const char* fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error(std::string("Failed to open file ") + fileName + " !");
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

static VkDeviceSize getRequiredAligment(VkDeviceSize buferSize, VkDeviceSize minAligment)
{
	VkDeviceSize requiredAligment = minAligment;

	while (requiredAligment < buferSize)
	{
		requiredAligment <<= 1;
	}

	return requiredAligment;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << '\n' << std::endl;

	return VK_FALSE;
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	static int count = 0;
	count++;
	VulkanBase* myApp = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
	myApp->m_swapchain.framebufferResized = true;
	App::Scene.m_camera.resizeView(width, height);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		App::Scene.m_camera.setRotateMode(true);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		App::Scene.m_camera.setRotateMode(false);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		App::Scene.m_simArea.clickEvent();
	}
}



static void cursorPosCallback(GLFWwindow* window, double width, double height)
{
	//App::Scene.m_grid.updateSelectedTile();
}

static inline glm::dvec2 getCursorPos(GLFWwindow* window)
{
	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);

	return { xPos, yPos };
}

static void keyboardInputCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		//App::Scene.m_simArea.roadCreator.rollBackEvent();
	}

	// weird
	//App::Scene.m_simArea.m_creator.processKeyInput(key, action);
}

VkResult CreteDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

void VulkanBase::populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo)
{
	messengerInfo = {};
	messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerInfo.flags = 0;
	messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messengerInfo.pfnUserCallback = debugCallback;
	messengerInfo.pUserData = nullptr;
}

void VulkanBase::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(CameraSettings::defaultWidth, CameraSettings::defaultHeight, "Sample title", nullptr, nullptr);

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
	glfwSetCursorPosCallback(m_window, cursorPosCallback);
	glfwSetKeyCallback(m_window, keyboardInputCallback);
}
void VulkanBase::run()
{
	initWindow();
	initVulkan();

	initUI();

	mainLoop();

	cleanup();
}

VkRenderPass VulkanBase::getRenderPass() const
{
	return m_renderPass;
}

uint16_t VulkanBase::getSwapchainImageCount() const
{
	return m_swapchain.images.size();
}

std::vector<vkh::structs::Buffer>& VulkanBase::getUniformBuffers()
{
	return m_buffers.uniform;
}

pGraphicsModule VulkanBase::getNewGraphicsModule()
{
	pGraphicsModule gModule =  m_graphicsModules.top();
	m_graphicsModules.pop();

	return gModule;
}

void VulkanBase::removeGraphicsModule(const pGraphicsModule& graphicsModule)
{
	/*auto gIt = std::find(std::begin(m_activeGraphicsModules), std::end(m_activeGraphicsModules), graphicsModule);
	m_activeGraphicsModules.erase(gIt);*/

	*graphicsModule = {};
	m_graphicsModules.push(graphicsModule);
}

GraphicsComponent VulkanBase::createGrahicsComponent(const Info::GraphicsComponentCreateInfo& info)
{
	pGraphicsModule gModule = createGraphicsModule(info);

	GraphicsComponent comp;
	comp.setGraphicsModule(gModule);

	return comp;
}

void VulkanBase::recreateGrahicsComponent(GraphicsComponent& gp, const Info::GraphicsComponentCreateInfo& info)
{
	removeGraphicsModule(&gp.getGraphicsModule());
	pGraphicsModule gModule = createGraphicsModule(info);

	gp.setGraphicsModule(gModule);

	// for uniform buffer
	if (gp.active)
		m_changedActiveComponentsSize = true;
}

void VulkanBase::copyGrahicsComponent(const GraphicsComponent& srcGraphicsComponent, GraphicsComponent& dstGraphicsComponent)
{
	if (srcGraphicsComponent.graphicsModule)
	{
		pGraphicsModule gModule = copyGraphicsModule(srcGraphicsComponent.getGraphicsModule());
		dstGraphicsComponent.setGraphicsModule(gModule);

		// for uniform buffer
		if (dstGraphicsComponent.active)
			m_changedActiveComponentsSize = true;
	}
}


void VulkanBase::destroyGraphicsComponent(const GraphicsComponent& comp)
{
	//m_dataManager.removeModelReference(comp.graphicsModule->pModelReference);

	removeGraphicsModule(comp.graphicsModule);
}

void VulkanBase::activateGraphicsComponent(GraphicsComponent* toActivate)
{
	m_activeGraphicsComponents.push_back(toActivate);
	
	m_changedActiveComponentsSize = true;
}

void VulkanBase::deactivateGraphicsComponent(GraphicsComponent* toActivate)
{
	// no error check
	auto gComp = std::find(std::begin(m_activeGraphicsComponents), std::end(m_activeGraphicsComponents), toActivate);
	if (gComp != std::end(m_activeGraphicsComponents))
	{
		m_activeGraphicsComponents.erase(gComp);

		m_changedActiveComponentsSize = true;
	}
}


pGraphicsModule VulkanBase::createGraphicsModule(const Info::GraphicsComponentCreateInfo& info)
{
	Model model;

	auto& modelVariant = info.modelInfo->model;
	if (auto pVal = std::get_if<std::string>(&modelVariant))
		model.loadModel(*pVal);
	else
		model = *std::get<Model*>(modelVariant);

	auto modelData = m_dataManager.loadModel(model);

	m_descriptorManager.processModelData(modelData);
	m_pipelinesManager.processModelData(modelData, *info.drawInfo);


	pGraphicsModule newModule = getNewGraphicsModule();
	newModule->modelData = modelData;

	return newModule;
}

pGraphicsModule VulkanBase::copyGraphicsModule(const GraphicsModule& copyModule)
{
	pGraphicsModule copiedModule = getNewGraphicsModule();
	copiedModule->modelData = copyModule.modelData;

	return copiedModule;
}

vkh::structs::VulkanDevice* VulkanBase::getDevice()
{
	return &m_device;
}

vkh::structs::Swapchain& VulkanBase::getSwapchain()
{
	return m_swapchain;
}

VkSampler& VulkanBase::getSampler()
{
	return m_sampler;
}

GLFWwindow* VulkanBase::getWindow()
{
	return m_window;
}

VkPushConstantRange& VulkanBase::getPushRange()
{
	return m_pushRange;
}

void VulkanBase::initVulkan()
{
	// basic
	createInstance();
	setupDebugMessenger();
	createSurface();
	createDevice();
	createPushRanges();

	// presentation
	createSwapchain();
	createTextureSampler();
//	createImages();

	//pipeline
	createRenderPass();
	//createDescriptorSetLayout();
//	createGraphicsPipeline();

	createDepthResources();
	createFramebuffers();
	createCommandPool();

	createSyncObjects();
	createPushRanges();

	initBuffers();
	initModules();

	createCommandBuffers();
}

void VulkanBase::initModules()
{
	m_dataManager.initialize(this);
	m_descriptorManager.init(this);
	m_pipelinesManager.init(this);


	initGraphicsMoudules();
}

void VulkanBase::initGraphicsMoudules()
{
	constexpr size_t maxGraphicsComponents = 100'000;
	//GraphicsComponent* gm = new GraphicsComponent[maxGraphicsComponents];
	//FIX THIS
	for (int index = 0; index < maxGraphicsComponents; ++index)
	{
		m_graphicsModules.push(new GraphicsModule);
	}
}

void VulkanBase::initUI()
{
	UI& instance = UI::getInstance();
	instance.initUI(this);
}

void VulkanBase::mainLoop()
{
	std::chrono::time_point lastFrame = std::chrono::high_resolution_clock::now();
	double deltaTime = 0.0;

	//just for testing puproses
	App::Scene.initComponents();

	while (!glfwWindowShouldClose(m_window))
	{
		App::time.tick();
		deltaTime = App::time.deltaTime();

		glfwSetWindowTitle(m_window, (std::string("Sample title ") + std::to_string(int(1.0 / deltaTime))).c_str());
		App::Scene.m_camera.update(deltaTime, getCursorPos(m_window));
		App::Scene.m_simArea.update();

		prepareFrame();
		drawFrame();
		processInput();
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	vkDeviceWaitIdle(m_device);
}

void VulkanBase::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested but not supported!");
	}
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = SAMPLE_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = SAMPLE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessenger(debugCreateInfo);
		instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugCreateInfo;
	}
	else
	{
		instanceInfo.enabledLayerCount = 0;
		instanceInfo.pNext = nullptr;
	}

	VkResult res;
	res = vkCreateInstance(&instanceInfo, nullptr, &m_instance);

	if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		throw std::runtime_error("Cannot find a compatible Vulkan ICD\n");
	}
	else if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}
}

void VulkanBase::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT debugInfo;
	populateDebugMessenger(debugInfo);

	VkResult res;
	res = CreteDebugUtilsMessengerEXT(m_instance, &debugInfo, nullptr, &m_debugMessenger);

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to setup debug messenger!");
	}
}

void VulkanBase::createSurface()
{
	VkResult res;
	res = glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void VulkanBase::createDevice()
{
	VkPhysicalDevice pickedDevice = VK_NULL_HANDLE;

	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("No physical device which supports Vulkan found!");
	}

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());

	pickedDevice = physicalDevices[0];

	m_device.initVulkanDevice(pickedDevice, m_surface);

	VkPhysicalDeviceFeatures features{};
	features.wideLines = VK_TRUE;
	features.fillModeNonSolid = VK_TRUE;
	features.samplerAnisotropy = VK_TRUE;
	features.vertexPipelineStoresAndAtomics = VK_TRUE;

	if (enableValidationLayers)
	{
		m_device.createLogicalDevice(features, deviceExtensions,
			validationLayers, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	}
	else
	{
		m_device.createLogicalDevice(features, deviceExtensions,
			{}, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	}
}

VkSurfaceFormatKHR VulkanBase::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats) const
{
	for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	// first be it
	return availableFormats[0];
}

VkPresentModeKHR VulkanBase::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) const
{
#if ENABLE_VSYNC
	return VK_PRESENT_MODE_FIFO_KHR;
#endif

	for (const VkPresentModeKHR availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanBase::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	VkExtent2D actualExtent{ width, height };

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void VulkanBase::createSwapchain()
{
	vkh::structs::SwapchainSupportDetails details = m_device.querySwapchainSupportDetails();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
	VkExtent2D extent = chooseSwapExtent(details.capabilities);

	uint32_t imageCount = std::clamp(details.capabilities.minImageCount,
		details.capabilities.minImageCount + 1, details.capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = m_surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (m_device.queueFamilyIndices.isSameFamily())
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = nullptr;
	}
	else
	{
		uint32_t queueFamilyIndices[]{ m_device.queueFamilyIndices.graphicsFamily.value(), m_device.queueFamilyIndices.presentFamily.value() };

		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	swapchainInfo.preTransform = details.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;

	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult res;
	res = vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain.swapchain);

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(m_device, m_swapchain.swapchain, &imageCount, nullptr);
	m_swapchain.images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain.swapchain, &imageCount, m_swapchain.images.data());

	m_swapchain.format = surfaceFormat.format;
	m_swapchain.extent = extent;
	m_swapchain.framebufferResized = false;

	// and create image views
	m_swapchain.imageViews.resize(m_swapchain.images.size());
	for (int i = 0; i < m_swapchain.images.size(); ++i)
	{
		m_device.createImageView(m_swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT,
			m_swapchain.images[i], m_swapchain.imageViews[i]);
	}
}

void VulkanBase::createImage(const char* filename, vkh::structs::Image& image) const
{
	int width, height, channels;
	unsigned char* imageData = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (!imageData)
	{
		throw std::runtime_error(std::string("Failed to load image ") + filename);
	}

	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	m_device.createImage(width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

	const VkDeviceSize uploadSize = width * height * 4;

	vkh::structs::Buffer uploadBuffer;
	m_device.createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		uploadSize, uploadBuffer, imageData);

	VkCommandBuffer copyBuffer = m_device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	image.transitionLayout(m_device, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyBuffer);

	VkBufferImageCopy region{};
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(copyBuffer, uploadBuffer, image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	image.transitionLayout(m_device, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, copyBuffer);
	m_device.flushCommandBuffer(copyBuffer, m_device.queues.graphics);

	uploadBuffer.cleanup(m_device, nullptr);
}


void VulkanBase::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapchain.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkResult res;
	res = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass!");
	}
}

void VulkanBase::recreateSwapchain()
{
	// minimalization
	throw std::runtime_error("DOnt minimalize or resize, thanks!");
	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(m_device);

	cleanupSwapchain();
	createSwapchain();
	createRenderPass();
	//createGraphicsPipeline();
	createDepthResources();
	createFramebuffers();
	
	initBuffers();

	createCommandBuffers();
}

void VulkanBase::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo = vkh::initializers::samplerCreateInfo(
		VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_TRUE, 16.0f
	);


	VK_CHECK_RESULT(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler));
}


void VulkanBase::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	VkImageAspectFlags depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT;

	m_device.createImage(m_swapchain.extent.width, m_swapchain.extent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthAspect,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, true);

	// transition for idk
	VkCommandBuffer copyBuffer = m_device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	m_depthImage.transitionLayout(m_device, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, copyBuffer);
	m_device.flushCommandBuffer(copyBuffer, m_device.queues.graphics);
}

VkFormat VulkanBase::findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : formats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR &&
			(properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
			(properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find suported format!");
}

VkFormat VulkanBase::findDepthFormat()
{
	static std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
	};

	return findSupportedFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void VulkanBase::createFramebuffers()
{
	m_swapchain.framebuffers.resize(m_swapchain.images.size());

	for (size_t i = 0; i < m_swapchain.images.size(); ++i)
	{
		std::array<VkImageView, 2> attachments { m_swapchain.imageViews[i], m_depthImage.view };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapchain.extent.width;
		framebufferInfo.height = m_swapchain.extent.height;
		framebufferInfo.layers = 1;

		VkResult res;
		res = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapchain.framebuffers[i]);
		if (res != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void VulkanBase::createCommandPool()
{
	m_commandPool = m_device.createCommandPool(m_device.queueFamilyIndices.graphicsFamily.value(),
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

void VulkanBase::initBuffers()
{
	m_buffers.uniform.resize(m_swapchain.images.size());
	for (auto& uniformBuffer : m_buffers.uniform)
	{
		createUniformBuffer(uniformBuffer, VulkanSettings::defaultUniformBufferSize);
	}
}


void VulkanBase::createVertexBuffer(vkh::structs::Buffer& vertexBuffer, VkDeviceSize bufferSize) const
{
	m_device.createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize, vertexBuffer);
}


void VulkanBase::createIndexBuffer(vkh::structs::Buffer& indexBuffer, VkDeviceSize bufferSize) const
{
	m_device.createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize, indexBuffer);
}

void VulkanBase::createUniformBuffer(vkh::structs::Buffer& uniformBuffer, VkDeviceSize bufferSize) const
{
	const int imageCount = m_swapchain.images.size();
	const VkDeviceSize defBuffSize = sizeof(UniformBufferObject);
	const VkDeviceSize dynamicAligment = 
		getRequiredAligment(defBuffSize, m_device.properties.limits.minUniformBufferOffsetAlignment);

	VkDeviceSize totalBufferSize = dynamicAligment * bufferSize;

	m_device.createBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, totalBufferSize, uniformBuffer);

}

void VulkanBase::createPushRanges()
{
	m_pushRange.size = sizeof(PushRange);
	m_pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	m_pushRange.offset = 0;
}

void VulkanBase::updateUniformBufferOffsets()
{
	auto strideSize = 
		getRequiredAligment(sizeof(UniformBufferObject), m_device.properties.limits.minUniformBufferOffsetAlignment);
	size_t totalBytes = 0;
	
	for (auto& graphicsComponent : m_activeGraphicsComponents)
		totalBytes = graphicsComponent->getGraphicsModule().modelData.meshDatas.size()* strideSize;

	// pseudo warning
	if (m_buffers.uniform[0].size < totalBytes)
		throw std::runtime_error("Not engough space!");
	VkDeviceSize offset = 0;
	for (auto& graphicsComponent : m_activeGraphicsComponents)
	{
		for (auto& meshData : graphicsComponent->getGraphicsModule().modelData.meshDatas)
		{
			meshData.dynamicBufferOffset = offset;
			offset += strideSize;
		}
	}

	m_changedActiveComponentsSize = false;
}

void VulkanBase::createCommandBuffers()
{
	m_commandBuffers.resize(m_swapchain.images.size());

	VkCommandBufferAllocateInfo cmdBufferAlloc{};
	cmdBufferAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAlloc.commandPool = m_commandPool;
	cmdBufferAlloc.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
	cmdBufferAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;


	VkResult res;
	res = vkAllocateCommandBuffers(m_device, &cmdBufferAlloc, m_commandBuffers.data());
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command buffers!");
	}
}

void VulkanBase::recreateCommandBuffer(uint32_t currentImage)
{
	auto& cmdBuffer = m_commandBuffers[currentImage];
	const VkDeviceSize dynamicAligment =
		getRequiredAligment(sizeof(UniformBufferObject), m_device.properties.limits.minUniformBufferOffsetAlignment);

	vkResetCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;

	VkResult res;
	res = vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin record command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchain.framebuffers[currentImage];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = m_swapchain.extent;

	std::array<VkClearValue, 2> clearValues;
	clearValues[0].color = { 0.5, 0.7, 1.0, 1.0 };
	clearValues[1].depthStencil = { 1, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	GO::ID graphicsPipeline = -1;
	GO::VertexType vertType = GO::VertexType::MAX_TYPE;
	VkDeviceSize vertexOffsets[] = { 0 };
	uint32_t descriptorOffsets[] = { 0 };

	// update constants
	m_pushConstants.projection = App::Scene.m_camera.getProjection();
	m_pushConstants.view = App::Scene.m_camera.getView();

	for (const auto& graphicsComponent : m_activeGraphicsComponents)
	{
		drawModelData(graphicsComponent->getGraphicsModule().modelData, cmdBuffer, currentImage);
	}

	// drawUI
	UI::getInstance().drawUI(cmdBuffer);
	vkCmdEndRenderPass(cmdBuffer);
	res = vkEndCommandBuffer(cmdBuffer);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void VulkanBase::drawModelData(const VD::ModelData& modelData, VkCommandBuffer& cmdBuff, uint32_t currentImage)
{
	VD::Pipeline* currentPipeline = nullptr;
	vkh::structs::Buffer* currentVertexBuffer = nullptr;
	vkh::structs::Buffer* currentIndexBuffer = nullptr;

	size_t vertexOffset = 0;
	size_t vertecCount = 0;
	size_t indexOffset = 0;
	size_t indexCount = 0;
	for (const auto& meshData : modelData.meshDatas)
	{
		if (&(*meshData.pipeline) != currentPipeline)
		{
			currentPipeline = &(*meshData.pipeline);

			vkCmdBindPipeline(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipeline);
			vkCmdPushConstants(cmdBuff, currentPipeline->pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(m_pushConstants), &m_pushConstants);
		}

		if (currentVertexBuffer != meshData.vertexBuffer)
		{
			currentVertexBuffer = meshData.vertexBuffer;

			VkBuffer vertexBuffers[] = { *currentVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmdBuff, 0, 1, vertexBuffers, offsets);

			const size_t vertexSize = VD::vertexSizeFromFlags(meshData.drawData.vertices->buffer.type);

			vertexOffset = meshData.drawData.vertices->byteOffset / vertexSize;
			vertecCount = meshData.drawData.vertices->buffer.data.size() / vertexSize;
		}

		if (currentIndexBuffer != meshData.indexBuffer && meshData.indexBuffer != nullptr)
		{
			currentIndexBuffer = meshData.indexBuffer;

			VkDeviceSize offsets[] = { 0 };
			vkCmdBindIndexBuffer(cmdBuff, *currentIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

			const size_t indexSize = sizeof(uint32_t);

			indexOffset = meshData.drawData.indices->byteOffset / indexSize;
			indexCount = meshData.drawData.indices->buffer.size();
		}

		VkDescriptorSet descriptorSets[] = { meshData.descriptorSet->sets[currentImage] };
		uint32_t offsets[] = { meshData.dynamicBufferOffset };
		vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipelineLayout, 0,
			1, descriptorSets, 1, offsets);

		if (meshData.indexBuffer)
			vkCmdDrawIndexed(cmdBuff, indexCount, 1, indexOffset, vertexOffset, 0);
		else
			vkCmdDraw(cmdBuff, vertecCount, 1, vertexOffset, 0);
	}
}

void VulkanBase::createSyncObjects()
{
	m_syncObjects.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGTH);
	m_syncObjects.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGTH);
	m_syncObjects.inFlightFences.resize(MAX_FRAMES_IN_FLIGTH);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGTH; ++i)
	{
		if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_syncObjects.imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_syncObjects.renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_device, &fenceInfo, nullptr, &m_syncObjects.inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create sync objects!");
		}
	}

	m_syncObjects.currentFrame = 0;
}

void VulkanBase::prepareFrame()
{
	if (m_changedActiveComponentsSize)
	{
		updateUniformBufferOffsets();
	}
}

void VulkanBase::drawFrame()
{
	vkWaitForFences(m_device, 1, &m_syncObjects.inFlightFences[m_syncObjects.currentFrame], VK_TRUE, UINT64_MAX);

	VkResult res;
	uint32_t imageIndex;
	res = vkAcquireNextImageKHR(m_device, m_swapchain.swapchain, UINT64_MAX,
		m_syncObjects.imageAvailableSemaphores[m_syncObjects.currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return;
	}
	else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swapchain image!");
	}

	recreateCommandBuffer(imageIndex);
	updateUniformBuffer(imageIndex);


	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[]{ m_syncObjects.imageAvailableSemaphores[m_syncObjects.currentFrame] };
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

	VkSemaphore signalSemaphore[]{ m_syncObjects.renderFinishedSemaphores[m_syncObjects.currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphore;

	vkResetFences(m_device, 1, &m_syncObjects.inFlightFences[m_syncObjects.currentFrame]);

	res = vkQueueSubmit(m_device.queues.graphics, 1, &submitInfo, m_syncObjects.inFlightFences[m_syncObjects.currentFrame]);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphore;

	VkSwapchainKHR swapchains[]{ m_swapchain.swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	res = vkQueuePresentKHR(m_device.queues.present, &presentInfo);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_swapchain.framebufferResized)
	{
		m_swapchain.framebufferResized = false;
		recreateSwapchain();
	}
	else if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swapchain image!");
	}

	m_syncObjects.currentFrame = (m_syncObjects.currentFrame + 1) % MAX_FRAMES_IN_FLIGTH;
}

void VulkanBase::updateUniformBuffer(uint32_t currentImage)
{
	static std::chrono::time_point startTime = std::chrono::high_resolution_clock::now();

	std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	const VkDeviceSize dynamicAligment = getRequiredAligment(sizeof(UniformBufferObject), 256);
	uint8_t* const data = static_cast<uint8_t* const>(m_buffers.uniform[currentImage].map());

	int count = 0;
	for (const auto& graphicsComponent : m_activeGraphicsComponents)
	{
		const auto graphicsModule = graphicsComponent->graphicsModule;

		for (const auto& meshData : graphicsModule->modelData.meshDatas)
		{
			UniformBufferObject ubo;
			auto model = glm::mat4(1.0);
			const auto& [position, rotation, size] = graphicsModule->transformations;

			model = glm::translate(model, position);
			model = glm::rotate(model, glm::radians(rotation.x), (glm::vec3)Transformations::VectorUp);
			model = glm::rotate(model, glm::radians(rotation.y), (glm::vec3)Transformations::VectorRight);
			model = glm::rotate(model, glm::radians(rotation.z), (glm::vec3)Transformations::VectorForward);
			model = glm::scale(model, size);
			ubo.model = model;
			ubo.tint = graphicsModule->shaderInfo.tint;
			ubo.transparency = graphicsModule->shaderInfo.transparency;

			memcpy(data + meshData.dynamicBufferOffset, &ubo, dynamicAligment);
		}
	}
	m_buffers.uniform[currentImage].unmap();
}


void VulkanBase::cleanup()
{
	UI::getInstance().destroyUI();
	destroyGraphicsComponents();
	cleanupSwapchain();
	cleanupBuffers();

	vkDestroySampler(m_device, m_sampler, nullptr);

	m_syncObjects.cleanUp(m_device, nullptr);
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	m_pipelinesManager.cleanup(nullptr);
	m_descriptorManager.cleanup(nullptr);
	m_device.destroyVulkanDevice();

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);

	glfwTerminate();
}

void VulkanBase::cleanupSwapchain()
{
	vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()),
		m_commandBuffers.data());

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);


	m_depthImage.cleanup(m_device, nullptr);
	m_swapchain.cleanup(m_device, nullptr);
}

void VulkanBase::cleanupBuffers()
{
	for (auto uniformBuffer : m_buffers.uniform)
		uniformBuffer.cleanup(m_device, nullptr);

	for (auto& [path, image] : m_images)
		image.cleanup(m_device, nullptr);
}

void VulkanBase::processInput()
{
	//App::Scene.m_simArea.m_creator.setCreateObject(selection);
	
	bool enableMouse = !UI::getInstance().mouseOverlap();
	App::Scene.m_simArea.setEnableMouse(enableMouse);
}

void VulkanBase::destroyGraphicsComponents()
{
	// not proper cleanup
	pGraphicsModule gModule;
	while (m_graphicsModules.size())
	{
		gModule = m_graphicsModules.top();
		m_graphicsModules.pop();

		delete gModule;
	}
}
