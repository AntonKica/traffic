#pragma once

#include <vulkan/vulkan.h>
#include "vulkanHelper/VulkanStructs.h"
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <map>

#include "VulkanInfo.h"

namespace VulkanData
{
	using PositionVertex = glm::vec3;
	using PositionVertices = std::vector<PositionVertex>;

	using ColorVertex = glm::vec4;
	using ColorVertices = std::vector<ColorVertex>;

	using NormalVertex = glm::vec3;
	using NormalVertices = std::vector<NormalVertex>;

	using TextureVertex = glm::vec2;
	using TextureVertices = std::vector<TextureVertex>;

	enum VertexType
	{
		POSITION = 1 << 0,
		COLOR = 1 << 1,
		NORMAL = 1 << 2,
		TEXTURE = 1 << 3,
	};
	using VertexFlags = uint32_t;

	size_t vertexSizeFromFlags(VertexFlags flags);
	VkVertexInputBindingDescription getBindingDescriptionFromFlags(VertexFlags flags);
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsFromFlags(VertexFlags flags);

	using Index = uint32_t;
	using Indices = std::vector<Index>;

	enum class TextureType
	{
		DIFFUSE,
		SPECULAR,
		AMBIENT,
		UNKNOWN,
		MAX_TEXTURES
	};
	static TextureType& operator++ (TextureType& tt)
	{
		tt = static_cast<TextureType>(static_cast<int>(tt));
		return tt;
	}
	using FilePath = std::string;
	using Textures = std::map<TextureType, FilePath>;

	struct Texture
	{
		std::string path;
		vkh::structs::Image image;
	};

	using Bytes = std::vector<std::byte>;

	struct ByteVertices
	{
		VertexFlags type;
		Bytes data;
	};

	template<class Type> struct OffsetBuffer
	{
		Type buffer;
		size_t byteOffset = 0;
	};
	using OffsetedByteVertices = OffsetBuffer<ByteVertices>;
	using SharedOffsetedByteVertices = std::shared_ptr<OffsetedByteVertices>;

	using OffsetedIndices = OffsetBuffer<Indices>;
	using SharedOffsetedIndices = std::shared_ptr<OffsetedIndices>;

	using SharedByteVertices = std::shared_ptr<ByteVertices>;
	using SharedIndices = std::shared_ptr<Indices>;
	using SharedTexture = std::shared_ptr<Texture>;

	struct DescriptorPool
	{
		VkDescriptorPool pool;
		Info::DescriptorPoolInfo info;

		operator VkDescriptorPool() { return pool; }
		operator VkDescriptorPool() const { return pool; }
	};
	using UniqueDescriptorPool = std::unique_ptr<DescriptorPool>;
	using pDescriptorPool = DescriptorPool*;

	struct DescriptorSetLayout
	{
		VkDescriptorSetLayout layout;
		Info::DescriptorSetLayoutInfo info;

		operator VkDescriptorSetLayout() { return layout; }
		operator VkDescriptorSetLayout() const { return layout; }
	};
	using SharedDescriptorSetLayout = std::shared_ptr<DescriptorSetLayout>;


	struct DescriptorSet
	{
		// via imageCOunt
		std::vector<VkDescriptorSet> sets;
		std::shared_ptr<DescriptorSetLayout> setLayout;
		// id layout?
		Info::DescriptorSetInfo info;
	};
	using SharedDescriptorSet = std::shared_ptr<DescriptorSet>;

	struct Pipeline
	{
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;

		Info::PipelineInfo info;
	};
	using SharedPipeline = std::shared_ptr<Pipeline>;

	struct MeshData
	{
		struct {
			SharedOffsetedByteVertices vertices;
			SharedOffsetedIndices indices;

			std::map<TextureType, SharedTexture> textures;
		} drawData;

		using ID = uint32_t;

		vkh::structs::Buffer* vertexBuffer = nullptr;
		vkh::structs::Buffer* indexBuffer = nullptr;
		SharedDescriptorSet descriptorSet;
		SharedPipeline pipeline = {};

		size_t dynamicBufferOffset = 0;
	};


	struct ModelData
	{
		std::vector<MeshData> meshDatas;
	};
}
namespace VD = VulkanData;

// definition
namespace VulkanData
{
	static VkVertexInputBindingDescription getBindingDescriptionFromFlags(VertexFlags flags)
	{
		VkVertexInputBindingDescription bindingDescription{};

		bindingDescription.binding = 0;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// position is always, allow only one vertex attribute
		bindingDescription.stride = sizeof(PositionVertex);
		if (flags & COLOR)
			bindingDescription.stride += sizeof(ColorVertex);
		else if (flags & TEXTURE)
			bindingDescription.stride += sizeof(TextureVertex);

		return bindingDescription;
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsFromFlags(VertexFlags flags)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		VkVertexInputAttributeDescription attribute;
		attribute.binding = 0;
		attribute.location = 0;
		attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute.offset = 0;

		attributeDescriptions.push_back(attribute);

		if (flags & COLOR)
		{
			attribute.binding = 0;
			attribute.location = 1;
			attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attribute.offset = sizeof(PositionVertex);

			attributeDescriptions.push_back(attribute);
		}
		else if (flags & TEXTURE)
		{
			attribute.binding = 0;
			attribute.location = 1;
			attribute.format = VK_FORMAT_R32G32_SFLOAT;
			attribute.offset = sizeof(PositionVertex);

			attributeDescriptions.push_back(attribute);
		}

		return attributeDescriptions;
	}

	static size_t vertexSizeFromFlags(VertexFlags flags)
	{
		size_t size = 0;
		if (flags & VertexType::POSITION)
			size += sizeof(PositionVertex);
		if (flags & VertexType::COLOR)
			size += sizeof(ColorVertex);
		if (flags & VertexType::NORMAL)
			size += sizeof(NormalVertex);
		if (flags & VertexType::TEXTURE)
			size += sizeof(TextureVertex);

		return size;
	}
}
