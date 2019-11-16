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
#include "resource_creator.h"
#include "GraphicsComponent.h"
#include "DataManager.h"
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
	static bool renderGrid = true;

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
		size_t currentFrame;

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
	// visuals
	GLFWwindow* m_window;
	UI m_selectionUI;
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
	DataManager m_dataManager;
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
		std::map<GO::VertexType, vkh::structs::Buffer> vertex;
		vkh::structs::Buffer index;

		std::vector<vkh::structs::Buffer> uniform;
	} m_buffers;

	std::stack<GraphicsComponent*>  m_graphicsComponents;
	std::vector<GraphicsComponent*> m_activeGraphicsComponents;
	bool createdGraphicsComponent = false;
	// helperFunctions
	friend void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	std::vector<const char*> getRequiredExtensions() const;
	bool checkValidationLayerSupport() const;
	std::vector<char> readFile(const char* fileName);
public:
	void run();

	vkh::structs::VulkanDevice* getDevice();
	GLFWwindow* getWindow();

	VkRenderPass getRenderPass() const;
	uint16_t getSwapchainImageCount() const;

	pGraphicsComponent createGrahicsComponent(const Info::GraphicsComponentCreateInfo& info);
	//GraphicsModule* copyGraphicsModule(const GraphicsModule* src);
	void destroyGraphicsComponent(const pGraphicsComponent& comp);
private:
	Info::DescriptorSetCreateInfo generateSetCreatInfo(const Info::GraphicsComponentCreateInfo& info,
		const ModelReference& modelRef) const;
	Info::PipelineInfo generatePipelineCreateInfo(const Info::GraphicsComponentCreateInfo& info,
		const ModelReference& modelRef, GO::ID descriptorSetRefID) const;
	GraphicsModule createGraphicsModule(const Info::GraphicsComponentCreateInfo& info);
	pGraphicsComponent getGraphicsComponent();
	void removeGraphicsComponent(const pGraphicsComponent& graphicsModule);

	// textureManager?
	const vkh::structs::Image* getImage(const std::string& loadPath);
	std::optional<const vkh::structs::Image*> findImage(const std::string& filePath);

private:
	// menu
	void initWindow();
	void initVulkan();
	void initModules();
	void initGraphicsComponents();
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
	
	void updateVertexBuffer();
	void updateIndexBuffer();

	void updateUniformBufferOffsets();
//	void updateTextures();

	void createTextureSamplers();
	void createTextureSampler(VkSampler& sampler) const;
	//drawing

	void createCommandBuffers();
	void recreateCommandBuffer(uint32_t currentImage);
	void createSyncObjects();

	void prepareFrame();
	void drawFrame();
	void updateUniformBuffer(uint32_t currentImage);
	// clear
	void cleanup();
	void cleanupSwapchain();
	void cleanupBuffers();
	void processInput();
	void destroyGraphicsComponents();
};

#endif // !VULKAN_BASE_H
