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
	size_t getVertexSize(VertexType type);

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
	using Vertices = std::vector<VariantVertex>;
	using TypedVertices = std::pair<VertexType, Vertices>;
	using Indices = std::vector<uint32_t>;


	struct Model
	{
		std::string directory;
		std::string file;

		TypedVertices typedVertices;
		std::optional<Indices> indices;
		std::optional<std::string> texturePath;

		static std::string getPath(const Model& model);
	};

	using ID = uint32_t;
	struct ModelReference
	{
		std::string file;

		ID vertices = 0;
		VertexType verticesType;
		std::optional<ID> indices;
		std::optional<ID> textureRefID;
	};
};
namespace GO = GraphicsObjects;