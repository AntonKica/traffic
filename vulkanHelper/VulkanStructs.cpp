#include "VulkanStructs.h"
#include <set>

bool vkh::structs::QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value() && presentFamily.has_value();
}

bool vkh::structs::QueueFamilyIndices::isSameFamily()
{
	return graphicsFamily.value() == presentFamily.value();
}

vkh::structs::Buffer::operator VkBuffer() const
{
	return buffer;
}

vkh::structs::Buffer::operator VkDeviceMemory() const
{
	return memory;
}

bool vkh::structs::Buffer::initialized() const
{
	return buffer != nullptr && memory != nullptr;
}

VkResult vkh::structs::Buffer::bind(VkDeviceSize offset)
{
	return vkBindBufferMemory(device, buffer, memory, offset);
}

void vkh::structs::Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
	info.buffer = buffer;
	info.range = size;
	info.offset = offset;
}

void* vkh::structs::Buffer::map(VkDeviceSize size, VkDeviceSize offset)
{
	vkMapMemory(device, memory, offset, size, 0, &mapped);
	return mapped;
}

void vkh::structs::Buffer::unmap()
{
	if (mapped)
	{
		vkUnmapMemory(device, memory);
		mapped = nullptr;
	}
}

void vkh::structs::Buffer::cleanup(VkDevice device, const VkAllocationCallbacks* allocator)
{
	if (buffer && memory)
	{
		vkDestroyBuffer(device, buffer, allocator);
		vkFreeMemory(device, memory, allocator);
	}
	buffer = nullptr;
	memory = nullptr;
	mapped = nullptr;
}

vkh::structs::Image::operator VkImage() const
{
	return image;
}

VkResult vkh::structs::Image::bind(VkDeviceSize offset)
{
	return vkBindImageMemory(device, image, memory, offset);
}

void vkh::structs::Image::setupDescriptor(VkSampler sampler)
{
	info.imageLayout = layout;
	info.imageView = view;
	info.sampler = sampler;
}

void vkh::structs::Image::cleanup(VkDevice device, const VkAllocationCallbacks* allocator)
{
	if (view)
	{
		vkDestroyImageView(device, view, allocator);
	}
	if (image && memory)
	{
		vkDestroyImage(device, image, allocator);
		vkFreeMemory(device, memory, allocator);
	}

	view = nullptr;
	image = nullptr;
	memory = nullptr;
	layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void vkh::structs::Image::transitionLayout(const vkh::structs::VulkanDevice& vkDevice, VkImageLayout newLayout, VkCommandBuffer cmdBuffer)
{
	VkImageMemoryBarrier barrier = vkh::initializers::imageMemoryBarrier();
	barrier.oldLayout = layout;
	barrier.newLayout = newLayout;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (layout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (layout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT
			;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::runtime_error("Unkown transition layout!");
	}

	vkCmdPipelineBarrier(
		cmdBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	layout = newLayout;
}

void vkh::structs::Swapchain::cleanup(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
	for (size_t i = 0; i < images.size(); ++i)
	{
		vkDestroyFramebuffer(device, framebuffers[i], pAllocator);
		vkDestroyImageView(device, imageViews[i], pAllocator);
	}

	vkDestroySwapchainKHR(device, swapchain, pAllocator);
}

vkh::structs::VulkanDevice::operator VkDevice() const { return logicalDevice; }

void vkh::structs::VulkanDevice::initVulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	if (!physicalDevice)
		throw std::runtime_error("Physical device is nullptr!");

	this->physicalDevice = physicalDevice;
	this->surface = surface;

	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	if (queueFamilyCount == 0)
		throw std::runtime_error("No queue families found!");

	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice, &queueFamilyCount, queueFamilies.data());

	uint32_t extCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
	if (extCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
			&extCount, extensions.data()) == VK_SUCCESS)
		{
			for (auto extension : extensions)
			{
				supportedExtensions.push_back((extension.extensionName));
			}
		}
	}
}

void vkh::structs::VulkanDevice::destroyVulkanDevice()
{
	if (commandPool)
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
	if (logicalDevice)
		vkDestroyDevice(logicalDevice, nullptr);
}

VkResult vkh::structs::VulkanDevice::createLogicalDevice(
	const VkPhysicalDeviceFeatures& enabledFeatures,
	const std::vector<const char*>& deviceExtensions,
	const std::vector<const char*>& validationLayers, 
	VkCommandPoolCreateFlags poolFlags)
{
	if (!isDeviceSuitable(deviceExtensions))
		throw std::runtime_error("Device is not suitable!");

	// create queueFamily
	queueFamilyIndices = getQueueFamilyIndices();

	std::set<uint32_t> uniqueQueueFamilies
	{
		queueFamilyIndices.graphicsFamily.value(),
		queueFamilyIndices.presentFamily.value()
	};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (uint32_t uniqueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount = 1;
		queueInfo.queueFamilyIndex = uniqueFamily;
		queueInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueInfo);
	}

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceInfo.pEnabledFeatures = &enabledFeatures;
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());;
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (!validationLayers.empty())
	{
		deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
	}

	VkResult res;
	res = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice);

	if (res == VK_SUCCESS)
	{
		commandPool = createCommandPool(queueFamilyIndices.graphicsFamily.value(), poolFlags);
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsFamily.value(), 0, &queues.graphics);
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.presentFamily.value(), 0, &queues.present);
	}

	return res;
}

vkh::structs::QueueFamilyIndices vkh::structs::VulkanDevice::getQueueFamilyIndices() const
{
	vkh::structs::QueueFamilyIndices queueIndices;

	for (uint32_t index = 0; index < static_cast<uint32_t>(queueFamilies.size()); index++)
	{
		if (queueFamilies[index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueIndices.graphicsFamily = index;
		}

		VkBool32 presentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentSupport);

		if (queueFamilies[index].queueFlags && presentSupport)
		{
			queueIndices.presentFamily = index;
		}

		if (queueIndices.isComplete())
		{
			return queueIndices;
		}
	}

	throw std::runtime_error("Could not find a matching queue family index");
}

bool vkh::structs::VulkanDevice::checkDeviceExtensionSupport(const std::vector<const char*>& deviceExtensions) const
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const VkExtensionProperties& availableExtension : availableExtensions)
	{
		requiredExtensions.erase(availableExtension.extensionName);
	}

	return requiredExtensions.empty();
}

bool vkh::structs::VulkanDevice::isDeviceSuitable(const std::vector<const char*>& deviceExtensions) const
{
	vkh::structs::QueueFamilyIndices indices = getQueueFamilyIndices();
	bool extensionsSupported = checkDeviceExtensionSupport(deviceExtensions);

	bool swapchainAdequate = false;
	if (extensionsSupported)
	{
		vkh::structs::SwapchainSupportDetails details = querySwapchainSupportDetails();
		swapchainAdequate = !details.formats.empty() && !details.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapchainAdequate;
}

vkh::structs::SwapchainSupportDetails vkh::structs::VulkanDevice::querySwapchainSupportDetails() const
{
	vkh::structs::SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (formatCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

uint32_t vkh::structs::VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}
	else
	{
		throw std::runtime_error("Could not find a matching memory type!");
	}
}

 //Creates new buffer, if is initialized, cleanups the buffer
VkResult vkh::structs::VulkanDevice::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, vkh::structs::Buffer& buffer, const void* data) const
{
	if (buffer.initialized())
	{
		buffer.cleanup(logicalDevice, nullptr);
	}
	buffer.device = logicalDevice;
	buffer.size = size;
	buffer.usage = usage;
	buffer.alignment = 0;

	VkBufferCreateInfo bufferInfo = vkh::initializers::bufferCreateInfo(usage, size);

	VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer.buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, buffer.buffer, &memRequirements);

	uint32_t memTypeIndex = getMemoryType(memRequirements.memoryTypeBits, properties);

	VkMemoryAllocateInfo memAlloc =
		vkh::initializers::memoryAllocateInfo(memRequirements.size, memTypeIndex);
	VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &buffer.memory));

	if (data)
	{
		buffer.map();

		memcpy(buffer.mapped, data, size);
		// FLUSH?
		buffer.unmap();
	}

	buffer.setupDescriptor();

	return buffer.bind();
}

vkh::structs::Buffer vkh::structs::VulkanDevice::createStaginBuffer(VkDeviceSize size, const void* data)
{
	vkh::structs::Buffer stagingBuffer;

	createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size, stagingBuffer, data);
	
	return stagingBuffer;
}

// copy via src buffer size
void vkh::structs::VulkanDevice::copyBuffer(const vkh::structs::Buffer& srcBuffer, const vkh::structs::Buffer& dstBuffer) const
{
	copyBuffer(srcBuffer, dstBuffer, srcBuffer.size);
}

void vkh::structs::VulkanDevice::copyBuffer(const vkh::structs::Buffer& srcBuffer, const vkh::structs::Buffer& dstBuffer, const VkDeviceSize size) const
{
	VkBufferCopy copyRegion{};
	copyRegion.size = size;

	copyBuffer(srcBuffer, dstBuffer, &copyRegion);
}

void vkh::structs::VulkanDevice::copyBuffer(const vkh::structs::Buffer& srcBuffer, const vkh::structs::Buffer& dstBuffer, const VkBufferCopy* copyRegion) const
{
	VkCommandBuffer cmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy bufferCopy{};
	if (copyRegion)
	{
		bufferCopy = *copyRegion;
	}
	else
	{
		bufferCopy.size = srcBuffer.size;
	}

	vkCmdCopyBuffer(cmdBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &bufferCopy);

	flushCommandBuffer(cmdBuffer, queues.graphics);
}

void vkh::structs::VulkanDevice::copyBufferToImage(const vkh::structs::Buffer& srcBuffer, const vkh::structs::Image& dstImage, uint32_t width, uint32_t height) const
{
	auto cmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = dstImage.aspect;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(cmdBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	flushCommandBuffer(cmdBuffer, queues.graphics);
}

VkCommandPool vkh::structs::VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags) const
{
	VkCommandPoolCreateInfo poolInfo =
		vkh::initializers::commandPoolCreateInfo(createFlags, queueFamilyIndex);

	VkCommandPool cmdPool;
	VK_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &cmdPool));

	return cmdPool;
}

VkCommandBuffer vkh::structs::VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin) const
{
	VkCommandBufferAllocateInfo	cmdBufAllocInfo =
		vkh::initializers::commandBufferAllocateInfo(commandPool, level, 1);

	VkCommandBuffer cmdBuffer;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocInfo, &cmdBuffer));

	if (begin)
	{
		beginCommandBuffer(cmdBuffer);
	}

	return cmdBuffer;
}

void vkh::structs::VulkanDevice::beginCommandBuffer(VkCommandBuffer cmdBuffer) const
{
	VkCommandBufferBeginInfo beginInfo = vkh::initializers::commandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
}

void vkh::structs::VulkanDevice::flushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, bool free) const
{
	if (cmdBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

	VkSubmitInfo submitInfo = vkh::initializers::submitInfo(&cmdBuffer, 1);

	// optionally create FENCE

	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	vkQueueWaitIdle(queue);

	if (free)
	{
		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &cmdBuffer);
	}
}

// RETURN TYPE

void vkh::structs::VulkanDevice::createImage(
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling, 
	VkImageUsageFlags usage,
	VkImageAspectFlags aspect,
	VkMemoryPropertyFlags properties,
	vkh::structs::Image& image,
	bool createView) const
{
	image.device = logicalDevice;
	image.format = format;
	image.aspect = aspect;

	VkImageCreateInfo imageInfo = vkh::initializers::imageCreateInfo();
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = { width, height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateImage(logicalDevice, &imageInfo, nullptr, &image.image));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logicalDevice, image.image, &memRequirements);
	uint32_t memTypeIndex = getMemoryType(memRequirements.memoryTypeBits, properties);

	VkDeviceMemory imageMemory;
	VkMemoryAllocateInfo memAlloc =
		vkh::initializers::memoryAllocateInfo(memRequirements.size, memTypeIndex);
	VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &image.memory));

	image.bind();
	if (createView)
	{
		createImageView(image);
	}

	image.setupDescriptor(nullptr);
}

void vkh::structs::VulkanDevice::createImageView(vkh::structs::Image& image) const
{
	VkImageViewCreateInfo viewInfo =
		vkh::initializers::imageViewCreateInfo(image, image.format, image.aspect);
	VK_CHECK_RESULT(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &image.view));
}

void vkh::structs::VulkanDevice::createImageView(VkFormat format, VkImageAspectFlags aspect, VkImage& image, VkImageView& view) const
{
	VkImageViewCreateInfo viewInfo =
		vkh::initializers::imageViewCreateInfo(image, format, aspect);
	VK_CHECK_RESULT(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &view));
}
