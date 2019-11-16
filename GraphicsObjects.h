#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <map>
#include <optional>
#include <variant>

namespace GraphicsObjects
{
	enum VertexType
	{
		DEFAULT		= 0x0,
		COLORED		= 0x1,
		TEXTURED	= 0x2,
		MAX_TYPE	= 0x3,
	};
	VertexType& operator++(VertexType& vt);
	uint32_t getVertexSize(VertexType type);

	VkVertexInputBindingDescription getBindingDescriptionFromType(VertexType type);
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsFromType(VertexType type);
	struct Vertex
	{
		glm::vec3 position;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};

	struct ColoredVertex : Vertex
	{
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};
	using ColoredVertices = std::vector<ColoredVertex>;

	struct TexturedVertex : Vertex
	{
		glm::vec2 texCoord;

		TexturedVertex() = default;
		TexturedVertex(float x, float y, float z, float s, float t)
		{
			position = { x, y, z };
			texCoord = { s, t };
		};
		static VkVertexInputBindingDescription getBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};
	template<typename V> bool equalVertex(V v1, V v2)
	{
		return v1.position == v2.position;
	}

	union VariantVertex
	{
		Vertex vertex;
		ColoredVertex coloredVertex;
		TexturedVertex texturedVertex;
	};

	//using 
	using VariantVertices = std::vector<VariantVertex>;
	using TypedVertices = std::pair<VertexType, VariantVertices>;
	using Indices = std::vector<uint32_t>;


	using ByteVector = std::vector<std::byte>;
	using ByteVertices =
		struct
	{
		VertexType type;
		ByteVector vertices;
	};
	template <typename container>
	ByteVector createByteContainer(const container& c1, size_t elementSize);
	ByteVertices transformTypedVerticesToBytes(const TypedVertices& tTverts);
	uint64_t getVerticesCountFromByteVertices(const ByteVertices* bVts);

	struct Model
	{
		std::optional<std::string> directory;

		ByteVertices vertices;
		std::optional<Indices> indices;
		std::optional<std::string> texturePath;

		static std::optional<std::string> getFile(const Model& model);
	};

	using ID = uint32_t;

	template<typename container>
	ByteVector createByteContainer(const container& c1, size_t elementSize)
	{
		ByteVector bytes(c1.size() * elementSize);
		std::byte* dataPtr = static_cast<std::byte*>(bytes.data());

		// copy
		for (const auto& element : c1)
		{
			std::memcpy(dataPtr, &element, elementSize);
			dataPtr += elementSize;
		}

		return bytes;
	}
};
namespace GO = GraphicsObjects;

namespace Comparators
{
	template<class container> constexpr bool sizeMemoryCompareLess(container c1, container c2)
	{
		if (c1.size() == c2.size())
			return std::memcmp(c1.data(), c2.data(), c1.size()) == 1;

		return c1.size() < c2.size();
	}
	template<class container> constexpr bool sizeMemoryCompareEqual(container c1, container c2)
	{
		if (c1.size() == c2.size())
			return std::memcmp(c1.data(), c2.data(), c1.size()) == 0;

		return false;
	}

	struct ByteVerticesCompLess
	{
		bool operator()(const GO::ByteVertices& rhs, const GO::ByteVertices& lhs) const
		{
			if (!(lhs.type < rhs.type))
			{
				return sizeMemoryCompareLess(rhs.vertices, lhs.vertices);
			}
			return false;
		}
	};
	struct ByteVerticesCompEqual
	{
		bool operator()(const GO::ByteVertices& rhs, const GO::ByteVertices& lhs) const
		{
			if (lhs.type == rhs.type)
			{
				return sizeMemoryCompareEqual(rhs.vertices, lhs.vertices);
			}
			return false;
		}
	};
	struct IndicesCompLess
	{
		bool operator()(const GO::Indices& lhs, const GO::Indices& rhs) const
		{
			return sizeMemoryCompareLess(lhs, rhs);
		}
	};
	struct IndicesCompEqual
	{
		bool operator()(const GO::Indices& rhs, const GO::Indices& lhs) const
		{
			return sizeMemoryCompareEqual(rhs, lhs);
		}
	};
}

