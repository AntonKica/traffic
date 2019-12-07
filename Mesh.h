#pragma once
#include <vulkan/vulkan.h>
#include "VulkanData.h"

#include <vector>
#include <optional>
#include <map>
#include <string>

#include <glm/glm.hpp>

struct Mesh
{
	struct
	{
		VD::PositionVertices positions;
		VD::ColorVertices	colors;
		VD::NormalVertices	normals;
		VD::TextureVertices textures;
	} vertices;

	VD::Indices indices;
	VD::Textures textures;
};
