#pragma once
#include "vulkanHelper.h"
#include <vector>
#include <optional>


namespace vkh
{
	namespace structs
	{
		// forward declaration
		struct VulkanDevice;

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete();

			bool isSameFamily();
		};

		struct Buffer
		{
			VkDevice device = nullptr;

			VkBuffer buffer = nullptr;
			VkDeviceMemory memory = nullptr;
			VkDescriptorBufferInfo info = {};
			VkBufferUsageFlags usage = {};
			VkDeviceSize size = 0;
			VkDeviceSize alignment = 0;
			void* mapped = nullptr;

			operator VkBuffer() const;
			operator VkDeviceMemory() const;

			bool initialized() const;

			VkResult bind(VkDeviceSize offset = 0);

			void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

			void* map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

			void unmap();

			void cleanup(VkDevice device, const VkAllocationCallbacks* allocator);
		};

		struct Image
		{
			VkDevice device = nullptr;

			VkImage image = nullptr;
			VkDeviceMemory memory = nullptr;
			VkImageView view = nullptr;
			VkFormat format;
			VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageAspectFlags aspect = 0;
			VkDescriptorImageInfo info = {};
			// usage? idk
			VkDeviceSize alignment = 0;
			VkDeviceSize size = 0;

			operator VkImage() const;

			VkResult bind(VkDeviceSize offset = 0);

			void setupDescriptor(VkSampler sampler);

			void cleanup(VkDevice device, const VkAllocationCallbacks* allocator);

			void transitionLayout(
				const vkh::structs::VulkanDevice& vkDevice,
				VkImageLayout newLayout,
				VkCommandBuffer cmdBuffer
			);
		};

		struct SwapchainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities = {};
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct Swapchain
		{
			VkSwapchainKHR swapchain;
			VkFormat format;
			VkExtent2D extent;

			std::vector<VkImage> images;
			std::vector<VkImageView> imageViews;
			std::vector<VkFramebuffer> framebuffers;

			bool framebufferResized;

			void cleanup(VkDevice device, const VkAllocationCallbacks* pAllocator);
		};
	}
}

namespace vkh
{
	namespace structs
	{
		struct VulkanDevice
		{
			VkPhysicalDevice physicalDevice;
			VkDevice logicalDevice;
			VkSurfaceKHR surface;

			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
			VkPhysicalDeviceFeatures enabledFeatures;
			VkPhysicalDeviceMemoryProperties memoryProperties;

			std::vector<VkQueueFamilyProperties> queueFamilies;
			vkh::structs::QueueFamilyIndices queueFamilyIndices;
			struct
			{
				VkQueue graphics;
				VkQueue present;
			} queues;

			std::vector<std::string> supportedExtensions;

			VkCommandPool commandPool;

			operator VkDevice() const;

			void initVulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

			void destroyVulkanDevice();

			VkResult createLogicalDevice(
				const VkPhysicalDeviceFeatures& enabledFeatures,
				const std::vector<const char*>& requestedExtensions,
				const std::vector<const char*>& validationLayers = {},
				VkCommandPoolCreateFlags poolFlags = 0);

			vkh::structs::QueueFamilyIndices getQueueFamilyIndices() const;

			bool checkDeviceExtensionSupport(const std::vector<const char*>& requestedExtensions) const;

			bool isDeviceSuitable(const std::vector<const char*>& requestedExtensions) const;

			vkh::structs::SwapchainSupportDetails querySwapchainSupportDetails() const;

			uint32_t getMemoryType(
				uint32_t typeBits,
				VkMemoryPropertyFlags properties,
				VkBool32* memTypeFound = nullptr) const;


			VkResult createBuffer(
				VkBufferUsageFlags usage,
				VkMemoryPropertyFlags properties,
				VkDeviceSize size,
				vkh::structs::Buffer& buffer,
				const void* data = nullptr) const;

			vkh::structs::Buffer createStaginBuffer(
				VkDeviceSize size,
				const void* data = nullptr);

			// copy via src buffer size
			void copyBuffer(
				const vkh::structs::Buffer& srcBuffer,
				const vkh::structs::Buffer& dstBuffer) const;

			void copyBuffer(
				const vkh::structs::Buffer& srcBuffer,
				const vkh::structs::Buffer& dstBuffer,
				const VkDeviceSize size) const;

			void copyBuffer(
				const vkh::structs::Buffer& srcBuffer,
				const vkh::structs::Buffer& dstBuffer,
				const VkBufferCopy* copyRegion) const;

			void copyBufferToImage(
				const vkh::structs::Buffer& srcBuffer,
				const vkh::structs::Image& dstImage,
				uint32_t width,
				uint32_t height
			) const;

			VkCommandPool createCommandPool(
				uint32_t queueFamilyIndex,
				VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;

			VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false) const;
			void beginCommandBuffer(VkCommandBuffer cmdBuffer) const;
			void flushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, bool free = true) const;

			// RETURN TYPE
			void createImage(
				uint32_t width,
				uint32_t height,
				VkFormat format,
				VkImageTiling tiling,
				VkImageUsageFlags usage,
				VkImageAspectFlags aspect,
				VkMemoryPropertyFlags properties,
				vkh::structs::Image& image,
				bool createView = true) const;

			void createImageView(vkh::structs::Image& image) const;
			void createImageView(VkFormat format, VkImageAspectFlags aspect, VkImage& image, VkImageView& view) const;
		};
	}
}