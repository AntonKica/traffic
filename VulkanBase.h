#ifndef VULKAN_BASE_H
#define VULKAN_BASE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <stack>
#include <optional>
#include <map>
#include <glm/glm.hpp>

#include "vulkanHelper/VulkanStructs.h"
#include "UI.h"

#include "Models.h"
#include "GraphicsObjects.h"
#include "GraphicsComponent.h"
#include "VulkanDataManager.h"
#include "DescriptorManager.h"
#include "PipelinesManager.h"

#define ENABLE_VSYNC false

using GraphicsObjects::ID;

namespace
{
	constexpr int WIDTH = 800;
	constexpr int HEIGHT = 600;
	constexpr int MAX_FRAMES_IN_FLIGTH = 3;

	const std::vector<const char*> validationLayers
	{
		"VK_LAYER_KHRONOS_validation",
		//"VK_LAYER_LUNARG_monitor",
	};

	const std::vector<const char*> deviceExtensions
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef _DEBUG
	constexpr bool enableValidationLayers = true; 
#else
	//constexpr bool enableValidationLayers = true;
	constexpr bool enableValidationLayers = false;
#endif // !

	struct IndexBuffer : vkh::structs::Buffer
	{
		int indexCount = 0;
	};
	struct drawObject
	{
		vkh::structs::Buffer vertexBuffer;
		IndexBuffer indexBuffer;
		std::vector<vkh::structs::Buffer> uniformBuffer;

		void cleanup(VkDevice device, const VkAllocationCallbacks* pAllocator)
		{
			vertexBuffer.cleanup(device, pAllocator);
			indexBuffer.cleanup(device, pAllocator);
			
			for (auto& buffer : uniformBuffer)
			{
				buffer.cleanup(device, pAllocator);
			}
		}
	};

	struct syncObjects
	{
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		size_t currentFrame = 0;

		void cleanUp(VkDevice device, const VkAllocationCallbacks* pAllocator)
		{
			if (!imageAvailableSemaphores.empty() && !renderFinishedSemaphores.empty() && !inFlightFences.empty())
			{
				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGTH; ++i)
				{
					vkDestroySemaphore(device, imageAvailableSemaphores[i], pAllocator);
					vkDestroySemaphore(device, renderFinishedSemaphores[i], pAllocator);
					vkDestroyFence(device, inFlightFences[i], pAllocator);
				}
			}
		}
	};


	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::vec4 tint;
		float transparency;
	};
}

namespace VulkanSettings
{
	constexpr size_t mb256 = 256'000'000; // 256 mb
	constexpr size_t defaultVertexBufferSize = mb256;
	constexpr size_t defaultIndexBufferSize = mb256;

	// without account of 
	constexpr size_t defaultUniformBufferSize = 1'000'000;
}

class VulkanBase
{
private:
	// visuals
	GLFWwindow* m_window;
	// members
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	vkh::structs::VulkanDevice m_device;

	VkSurfaceKHR m_surface;
	vkh::structs::Swapchain m_swapchain;
	vkh::structs::Image m_depthImage;

	VkRenderPass m_renderPass;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	syncObjects m_syncObjects;

	//new parts
	VulkanDataManager m_dataManager;
	DescriptorManager m_descriptorManager;
	PipelinesManager m_pipelinesManager;

	VkSampler m_sampler;
	std::map<std::string, vkh::structs::Image> m_images;
	VkPushConstantRange m_pushRange;
	struct PushRange
	{
		glm::mat4 view;
		glm::mat4 projection;
	};
	PushRange m_pushConstants;
	
	struct
	{
		std::vector<vkh::structs::Buffer> uniform;
	} m_buffers;

	bool m_changedActiveComponentsSize = false;
	std::vector<GraphicsComponent*> m_activeGraphicsComponents;


	// helperFunctions
	friend void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	std::vector<const char*> getRequiredExtensions() const;
	bool checkValidationLayerSupport() const;
	std::vector<char> readFile(const char* fileName);
public:
	void run();

	vkh::structs::VulkanDevice* getDevice();
	vkh::structs::Swapchain& getSwapchain();
	VkSampler& getSampler();
	GLFWwindow* getWindow();
	VkPushConstantRange& getPushRange();

	VkRenderPass getRenderPass() const;
	uint16_t getSwapchainImageCount() const;
	std::vector<vkh::structs::Buffer>& getUniformBuffers();

	GraphicsComponent createGrahicsComponent(const Info::GraphicsComponentCreateInfo& info);
	void recreateGrahicsComponent(GraphicsComponent& graphicsComponent, const Info::GraphicsComponentCreateInfo& info);

	void activateGraphicsComponent(GraphicsComponent* toActivate);
	void deactivateGraphicsComponent(GraphicsComponent* toDeactivate);

private:
	VD::ModelData getModelDataFromInfo(const Info::GraphicsComponentCreateInfo& info);
private:
	// menu
	void initWindow();
	void initVulkan();
	void initModules();
	void initUI();
	void mainLoop();
	// Foundation
	// create instance
	void createInstance();
	void setupDebugMessenger();
	void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo);
	// physicalDevice
	void createSurface();
	// logical device
	void createDevice();

	// swapchain
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats) const;
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) const;
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	void createSwapchain();
	// imageViews
	void createImage(const char* filename, vkh::structs::Image& image) const;
	// pipeline
	void createRenderPass();
	// recreate
	void recreateSwapchain();


	// drawing
	void createDepthResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	void createFramebuffers();
	void createCommandPool();

	// default funcion to init vertex and indexBuffer
	void initBuffers();
	void createVertexBuffer(vkh::structs::Buffer& vertexBuffer, VkDeviceSize bufferSize) const;
	void createIndexBuffer(vkh::structs::Buffer& indexBuffer, VkDeviceSize bufferSize) const;
	void createUniformBuffer(vkh::structs::Buffer& uniformBuffer, VkDeviceSize bufferSize) const;

	void createPushRanges();
	//

	void updateUniformBufferOffsets();
//	void updateTextures();

	void createTextureSampler();
	//drawing

	void createCommandBuffers();
	void recreateCommandBuffer(uint32_t currentImage);
	void drawModelData(const VD::ModelData& modelData, VkCommandBuffer& cmdBuff, uint32_t currentImage);

	void createSyncObjects();

	void prepareFrame();
	void drawFrame();
	void updateUniformBuffer(uint32_t currentImage);
	// clear
	void cleanup();
	void cleanupSwapchain();
	void cleanupBuffers();
	void processInput();
};

#endif // !VULKAN_BASE_H
