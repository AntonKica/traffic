#pragma once
#include "BasicGeometry.h"
#include "VulkanData.h"
#include "polyPartition/earcut.hpp"

namespace EarcutAdaptation
{
	namespace details
	{
		struct ProcessedVertices
		{
			VD::PositionVertices positionVertices;
			VD::Indices indices;
		};
	}

	static details::ProcessedVertices triangulatePoints(const Points& points)
	{
		using EarCutPoint = std::array<float, 2>;

		// here we store result
		std::vector<std::vector<EarCutPoint>> polygon(1);
		// make space
		polygon[0].resize(points.size());
		//
		std::transform(std::begin(points), std::end(points), polygon[0].begin(),
			[](const Point& point) ->EarCutPoint
			{
				return { point.x, point.z };
			});

		// make indices
		std::vector<uint32_t> indices = mapbox::earcut(polygon);
		
		details::ProcessedVertices procVerts;
		procVerts.positionVertices = points;
		procVerts.indices = indices;

		return procVerts;
	}
}