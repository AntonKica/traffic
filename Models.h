#ifndef MODELS_H
#define MODELS_H

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>
#include "GraphicsObjects.h"

namespace Models
{
	using namespace GraphicsObjects;
	struct
	{
		const std::vector<GraphicsObjects::Vertex> vertices
		{
			{{-0.5f, 0.0f,-0.5f}},
			{{ 0.5f, 0.0f,-0.5f}},
			{{ 0.5f, 0.0f, 0.5f}},
			{{-0.5f, 0.0f, 0.5f}}
		};

		const std::vector<uint16_t> indices
		{
			2,1,0,
			0,3,2
		};

	} XZPlane;

	struct
	{
		const std::vector<GraphicsObjects::Vertex> vertices
		{
			{{-0.5f,-0.5f, 0.0f}},
			{{ 0.5f,-0.5f, 0.0f}},
			{{ 0.5f, 0.5f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}}
		};

		const std::vector<uint16_t> indices
		{
			2,1,0,
			0,3,2
		};

	} XYPlane;

	struct
	{
		const std::vector<GraphicsObjects::Vertex> vertices
		{
			{{-0.5f, 0.0f, 0.0f}},
			{{ 0.5f, 0.0f, 0.0f}},
			{{ 0.0f, 0.0f,-0.5f}},
			{{ 0.0f, 0.0f, 0.5f}},
		};

		const std::vector<uint16_t> indices
		{
			0,1,
			2,3
		};

	} XZCross;

	struct
	{
		const std::vector<GraphicsObjects::Vertex> vertices
		{
			{{-0.5f, 0.0f, 0.0f}},
			{{ 0.5f, 0.0f, 0.0f}},
		};
	} XLine;

	struct
	{
		const std::vector<GraphicsObjects::Vertex> vertices
		{
			{{ 0.0f, 0.0f, -0.5f}},
			{{ 0.0f, 0.0f, 0.5f}}
		};
	} ZLine;
}

#endif //!MODELS_H