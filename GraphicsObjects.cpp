#include "GraphicsObjects.h"

VkVertexInputBindingDescription GO::Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> GO::Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	return attributeDescriptions;
}

VkVertexInputBindingDescription GO::ColoredVertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(ColoredVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> GO::ColoredVertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(ColoredVertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(ColoredVertex, color);

	return attributeDescriptions;
}

VkVertexInputBindingDescription GO::TexturedVertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(TexturedVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> GO::TexturedVertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(TexturedVertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(TexturedVertex, texCoord);

	return attributeDescriptions;
}

std::string GO::Model::getPath(const Model& model)
{
	return model.directory.empty() ? model.file : model.directory + '/' + model.file;
}

size_t GO::getVertexSize(VertexType type)
{
	switch (type)
	{
	case GraphicsObjects::DEFAULT:
		return sizeof(Vertex);
	case GraphicsObjects::COLORED:
		return sizeof(ColoredVertex);
	case GraphicsObjects::TEXTURED:
		return sizeof(TexturedVertex);
	default:
		throw std::runtime_error("Unknown vertex type!");
	}
}

GO::VertexType& GraphicsObjects::operator++(VertexType& vt)
{
	vt = static_cast<VertexType>(static_cast<int>(vt) + 1);
	return vt;
}

VkVertexInputBindingDescription GraphicsObjects::getBindingDescriptionFromType(VertexType type)
{
	switch (type)
	{
	case GraphicsObjects::DEFAULT:
		return Vertex::getBindingDescription();
	case GraphicsObjects::COLORED:
		return ColoredVertex::getBindingDescription();
	case GraphicsObjects::TEXTURED:
		return TexturedVertex::getBindingDescription();
	default:
		throw std::runtime_error("Unknown vertex type!");
	}
}

std::vector<VkVertexInputAttributeDescription> GraphicsObjects::getAttributeDescriptionsFromType(VertexType type)
{
	switch (type)
	{
	case GraphicsObjects::DEFAULT:
		return Vertex::getAttributeDescriptions();
	case GraphicsObjects::COLORED:
		return ColoredVertex::getAttributeDescriptions();
	case GraphicsObjects::TEXTURED:
		return TexturedVertex::getAttributeDescriptions();
	default:
		throw std::runtime_error("Unknown vertex type!");
	}
}
