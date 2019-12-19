#pragma once
//#ifndef VULKAN_HELPER_H
//#define VULKAN_HELPER_H
#include <vulkan/vulkan.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <assert.h>

static std::string errorString(VkResult result)
{
	switch (result)
	{
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_FRAGMENTATION_EXT:
		return "VK_ERROR_FRAGMENTATION_EXT";
	case VK_ERROR_NOT_PERMITTED_EXT:
		return "VK_ERROR_NOT_PERMITTED_EXT";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
		return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_RESULT_RANGE_SIZE:
		return "VK_RESULT_RANGE_SIZE";
	case VK_RESULT_MAX_ENUM:
	default:
		return "UNKNOWN";
	}
}

#define VK_CHECK_RESULT(f)															\
{																					\
	VkResult res = (f);																\
	if (res != VK_SUCCESS)															\
	{																				\
		std::cout << "Fatal : VkResult is \"" <<errorString(res) <<					\
		"\" in " << __FILE__ << " at line " << __LINE__ << std::endl;				\
		assert(res == VK_SUCCESS);													\
	}																				\
}																					\

namespace errors
{
	struct ErrorCode
	{
		enum class code
		{
			NO_ERROR,
			FAILED_TO_OPEN_FILE,
			FAILED_TO_CREATE
		};

		code m_code = code::NO_ERROR;
		operator bool() const
		{
			return static_cast<int>(m_code);
		}
		ErrorCode& operator=(code cd) 
		{
			m_code = cd; 
			return *this;
		}
		bool operator==(code cd) 
		{
			return m_code == cd;
		}
	};
}

namespace loaders
{
	static std::vector<char> loadFileBuffer(const char* filepath, errors::ErrorCode& ec)
	{
		std::vector<char> fileBuffer;

		std::ifstream openFile(filepath, std::ios::ate | std::ios::binary);
		if (!openFile.is_open())
		{
			std::cerr << "Failed to open file " << filepath << '\n';
			ec = errors::ErrorCode::code::FAILED_TO_OPEN_FILE;
		}			
		else	
		{			
			size_t fileSize = openFile.tellg();
			fileBuffer.resize(fileSize);

			openFile.seekg(0, std::ios::beg);
			openFile.read(fileBuffer.data(), fileSize);
		}

		return fileBuffer;
	}
}

namespace vkh
{
	namespace initializers
	{
		// IMAGES
		inline VkImageCreateInfo imageCreateInfo()
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

			return imageInfo;
		}

		inline VkImageViewCreateInfo imageViewCreateInfo(
			VkImage image,
			VkFormat format,
			VkImageAspectFlags aspectMask
		)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;

			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			viewInfo.subresourceRange.aspectMask = aspectMask;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			return viewInfo;
		}

		inline VkImageMemoryBarrier imageMemoryBarrier()
		{
			VkImageMemoryBarrier memBarrier{};
			memBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			return memBarrier;
		}

		inline VkSamplerCreateInfo samplerCreateInfo(
			VkFilter magMinFilter,
			VkSamplerAddressMode adressMode,
			VkSamplerMipmapMode mipmapMode,
			VkBool32 enableAnistropy = VK_FALSE,
			float maxAnistropy = 0
		)
		{
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;

			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			samplerInfo.anisotropyEnable = enableAnistropy;
			samplerInfo.maxAnisotropy = maxAnistropy;

			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;

			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			return samplerInfo;
		}
		// BUFFERS
		inline VkBufferCreateInfo bufferCreateInfo(
			VkBufferUsageFlags usage,
			VkDeviceSize size,
			VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			VkBufferCreateFlags flags = 0,
			uint32_t queueIndexCount = 0,
			const uint32_t* queueIndex = nullptr
		)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.usage = usage;
			bufferInfo.size = size;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			bufferInfo.flags = 0;
			bufferInfo.queueFamilyIndexCount = queueIndexCount;
			bufferInfo.pQueueFamilyIndices = queueIndex;

			return bufferInfo;
		}

		inline VkBufferCopy bufferCopy(
			VkDeviceSize size,
			VkDeviceSize srcOffset = 0,
			VkDeviceSize dstOffset = 0
			)
		{
			VkBufferCopy bufferCopy{};
			bufferCopy.size = size;
			bufferCopy.srcOffset = srcOffset;
			bufferCopy.dstOffset = dstOffset;

			return bufferCopy;
		}

		//  COMMAND BUFFERS
		inline VkCommandPoolCreateInfo commandPoolCreateInfo(
			VkCommandPoolCreateFlags flags,
			uint32_t queueIndex
		)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = flags;
			poolInfo.queueFamilyIndex = queueIndex;

			return poolInfo;
		}

		inline VkMemoryAllocateInfo memoryAllocateInfo(
			VkDeviceSize allocSize,
			uint32_t memoryIndex
		)
		{
			VkMemoryAllocateInfo memInfo{};
			memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memInfo.allocationSize = allocSize;
			memInfo.memoryTypeIndex = memoryIndex;

			return memInfo;
		}

		inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
			VkCommandPool pool,
			VkCommandBufferLevel level,
			uint32_t cmdBufferCount
		)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = pool;
			allocInfo.level = level;
			allocInfo.commandBufferCount = cmdBufferCount;

			return allocInfo;
		}

		inline VkCommandBufferBeginInfo commandBufferBeginInfo(
			VkCommandBufferUsageFlags usage = 0
		)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = usage;

			return beginInfo;
		}

		inline VkSubmitInfo submitInfo(
			VkCommandBuffer* cmdBuffers, uint32_t cmdBufferCount,
			uint32_t waitSemaphoresCount = 0,
			const VkSemaphore* waitSemaphores = nullptr,
			const VkPipelineStageFlags* waitDstStageMask = nullptr,
			uint32_t signalSemaphoresCount = 0,
			const VkSemaphore* signalSemaphores = nullptr
			)
		{
			VkSubmitInfo subInfo{};
			subInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			subInfo.waitSemaphoreCount = waitSemaphoresCount;
			subInfo.pWaitSemaphores = waitSemaphores;
			subInfo.pWaitDstStageMask = waitDstStageMask;
			subInfo.pCommandBuffers = cmdBuffers;
			subInfo.commandBufferCount = cmdBufferCount;
			subInfo.pSignalSemaphores = signalSemaphores;
			subInfo.signalSemaphoreCount = signalSemaphoresCount;

			return subInfo;
		}
		// DESCCRIPTORS
		inline VkDescriptorPoolSize descriptorPoolSize(
			uint32_t desciptorCount,
			VkDescriptorType descriptorType
		)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.descriptorCount = desciptorCount;
			poolSize.type = descriptorType;

			return poolSize;
		}

		inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
			uint32_t poolCount,
			const VkDescriptorPoolSize* poolSizes,
			uint32_t maxSets
		)
		{
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = poolCount;
			poolInfo.pPoolSizes = poolSizes;
			poolInfo.maxSets = maxSets;

			return poolInfo;
		}

		inline VkDescriptorSetLayoutBinding descritptorSetLayoutBinding(
			VkDescriptorType descriptorType,
			VkShaderStageFlags shaderStage,
			uint32_t binding,
			const VkSampler* immutableSamplers = VK_NULL_HANDLE
		)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding;
			layoutBinding.descriptorType = descriptorType;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = shaderStage;
			layoutBinding.pImmutableSamplers = immutableSamplers;

			return layoutBinding;
		}

		inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			uint32_t layoutsCount,
			const VkDescriptorSetLayoutBinding* setLayouts
		)
		{
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = layoutsCount;
			layoutInfo.pBindings = setLayouts;
			layoutInfo.flags = 0;

			return layoutInfo;
		}

		inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
			VkDescriptorPool descriptorPool,
			uint32_t descriptorSetCount,
			VkDescriptorSetLayout* descirpotrSetsLayout
		)
		{
			VkDescriptorSetAllocateInfo descriptorAllocInfo{};
			descriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorAllocInfo.descriptorPool = descriptorPool;
			descriptorAllocInfo.descriptorSetCount = descriptorSetCount;
			descriptorAllocInfo.pSetLayouts = descirpotrSetsLayout;

			return descriptorAllocInfo;
		}

		inline VkDescriptorBufferInfo descriptorBufferInfo
		(
			VkBuffer buffer,
			VkDeviceSize offset,
			VkDeviceSize range
		)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			return bufferInfo;
		}

		inline VkWriteDescriptorSet writeDescriptorSet()
		{
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

			return descriptorWrite;
		}
		// RENDERPASS
		inline VkRenderPassCreateInfo renderPassCreateInfo(
			uint32_t attachmentsCount,
			const VkAttachmentDescription* attachments,
			uint32_t subpassCount,
			const VkSubpassDescription* subpasses,
			uint32_t dependencyCount,
			const VkSubpassDependency* dependencies
		)
		{
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = attachmentsCount;
			renderPassInfo.pAttachments = attachments;
			renderPassInfo.subpassCount = subpassCount;
			renderPassInfo.pSubpasses = subpasses;
			renderPassInfo.dependencyCount = dependencyCount;
			renderPassInfo.pDependencies = dependencies;

			return renderPassInfo;
		}
		// pipeline
		inline VkShaderModuleCreateInfo shaderModuleCreateInfo(
			uint32_t codeSize,
			const uint32_t* code
			)
		{
			VkShaderModuleCreateInfo shaderInfo{};
			shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderInfo.codeSize = codeSize;
			shaderInfo.pCode = code;

			return shaderInfo;
		}
		inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(
			VkShaderStageFlagBits stage,
			VkShaderModule module,
			const char* entryPoint = "main"
		)
		{
			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = stage;
			shaderStageInfo.module = module;
			shaderStageInfo.pName = entryPoint;

			return shaderStageInfo;
		}

		inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
			uint32_t bindingCount,
			const VkVertexInputBindingDescription* bindingDescription,
			uint32_t attributeCount,
			const VkVertexInputAttributeDescription* attributeDescriptions
		)
		{
			VkPipelineVertexInputStateCreateInfo inputInfo{};
			inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			inputInfo.vertexBindingDescriptionCount = bindingCount;
			inputInfo.pVertexBindingDescriptions = bindingDescription;
			inputInfo.vertexAttributeDescriptionCount = attributeCount;
			inputInfo.pVertexAttributeDescriptions = attributeDescriptions;

			return inputInfo;
		}
		inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			VkPrimitiveTopology topology,
			VkBool32 primitiveRestart
		)
		{
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
			inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyInfo.topology = topology;
			inputAssemblyInfo.primitiveRestartEnable = primitiveRestart;

			return inputAssemblyInfo;
		}

		inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
			uint32_t scissorCount,
			const VkRect2D* scissors,
			uint32_t viewportCount,
			const VkViewport* viewports
		)
		{
			VkPipelineViewportStateCreateInfo viewportInfo{};
			viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportInfo.scissorCount = scissorCount;
			viewportInfo.pScissors = scissors;
			viewportInfo.viewportCount = viewportCount;
			viewportInfo.pViewports = viewports;

			return viewportInfo;
		}

		inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
			VkBool32 depthClamp,
			VkBool32 rasterizationDiscard,
			VkPolygonMode polygonMode,
			float lineWidth,
			VkCullModeFlags cullMode,
			VkFrontFace frontFace,
			VkBool32 depthBias = VK_FALSE,
			float depthBiasConstantFactor = 0.0f,
			float depthBiasClamp = 0.0f,
			float depthBiasSlopeFactor = 0.0f
			)
		{
			VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
			rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationInfo.depthClampEnable = depthClamp;
			rasterizationInfo.rasterizerDiscardEnable = rasterizationDiscard;
			rasterizationInfo.polygonMode = polygonMode;
			rasterizationInfo.lineWidth = lineWidth;
			rasterizationInfo.cullMode = cullMode;
			rasterizationInfo.frontFace = frontFace;
			rasterizationInfo.depthBiasEnable = depthBias;
			rasterizationInfo.depthBiasConstantFactor = depthBiasConstantFactor;
			rasterizationInfo.depthBiasClamp = depthBiasClamp;
			rasterizationInfo.depthBiasSlopeFactor = depthBiasSlopeFactor;

			return rasterizationInfo;
		}

		inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
			VkBool32 sampleShadingEnable,
			VkSampleCountFlagBits sampleCount,
			float minSampleShading,
			const VkSampleMask* sampleMask,
			VkBool32 alphaToCoverageEnable = VK_FALSE,
			VkBool32 alphaToOneEnableEnable = VK_FALSE
		)
		{
			VkPipelineMultisampleStateCreateInfo multisampleInfo{};
			multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleInfo.sampleShadingEnable = sampleShadingEnable;
			multisampleInfo.rasterizationSamples = sampleCount;
			multisampleInfo.minSampleShading = minSampleShading;
			multisampleInfo.pSampleMask = sampleMask;
			multisampleInfo.alphaToCoverageEnable = alphaToCoverageEnable;
			multisampleInfo.alphaToOneEnable = alphaToOneEnableEnable;

			return multisampleInfo;
		}

		inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
			VkBool32 logicOpEnable,
			VkLogicOp logicOp,
			uint32_t attachmentsCount,
			VkPipelineColorBlendAttachmentState* attachments,
			float blendConstantR = 0.0f,
			float blendConstantG = 0.0f,
			float blendConstantB = 0.0f,
			float blendConstantA = 0.0f
			)
		{
			VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
			colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendInfo.logicOpEnable = logicOpEnable;
			colorBlendInfo.logicOp = logicOp;
			colorBlendInfo.attachmentCount = attachmentsCount;
			colorBlendInfo.pAttachments = attachments;
			colorBlendInfo.blendConstants[0] = blendConstantR;
			colorBlendInfo.blendConstants[1] = blendConstantG;
			colorBlendInfo.blendConstants[2] = blendConstantB;
			colorBlendInfo.blendConstants[2] = blendConstantA;

			return colorBlendInfo;
		}

		inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo()
		{
			VkPipelineDepthStencilStateCreateInfo depthInfo{};
			depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

			return depthInfo;
		}
		inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			uint32_t pushConstantCount,
			const VkPushConstantRange* pushConstants,
			uint32_t setLayoutCount,
			const VkDescriptorSetLayout* setLayouts
		)
		{
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pushConstantRangeCount = pushConstantCount;
			pipelineLayoutInfo.pPushConstantRanges = pushConstants;
			pipelineLayoutInfo.setLayoutCount = setLayoutCount;
			pipelineLayoutInfo.pSetLayouts = setLayouts;
			pipelineLayoutInfo.pushConstantRangeCount = pushConstantCount;

			return pipelineLayoutInfo;
		}

		inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo()
		{
			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

			return pipelineInfo;
		}
	}

	// baad
	namespace finders
	{

	}
}
//#endif // ! VULKAN_HELPER_H
