#pragma once
#include "Utilities.h"
#include <algorithm>

namespace Collisions
{
	template <class T> struct MinMax
	{
		T min, max;
	};

	template <class PointsContainer> static MinMax<float> projectPolygonOnAxis(const PointsContainer& polygonPoints, const glm::vec3& axis)
	{
		float dotProduct = glm::dot(axis, polygonPoints[0]);
		float min = dotProduct;
		float max = dotProduct;
		
		for (const auto& point : polygonPoints)
		{
			dotProduct = glm::dot(axis, point);
			min = std::min(min, dotProduct);
			max = std::max(max, dotProduct);
		}

		return MinMax<float>{ min, max };
	}

	static float intervalDistance(const MinMax<float>& firstInterval, const MinMax<float>& secondInterval)
	{
		if (firstInterval.min < secondInterval.min)
			return secondInterval.min - firstInterval.max;
		else
			return firstInterval.min - secondInterval.max;
	}


	static bool triangleTriangleCollision(const Triangle& triangleOne, const Triangle& triangleTwo)
	{
		bool canDrawLineBetween = false;

		// with first triangle
		for (uint32_t index = 0, nextIndex = 1; index < triangleOne.size(); ++index, ++nextIndex)
		{
			if (canDrawLineBetween)
				break;

			if (nextIndex == triangleOne.size())
				nextIndex = 0;

			const glm::vec3 edge = triangleOne[index] - triangleOne[nextIndex];

			const glm::vec3 vectorUp(0.0, 1.0, 0.0);
			const auto perpAxis = glm::normalize(glm::cross(edge, vectorUp));

			const auto firstInterval = projectPolygonOnAxis(triangleOne, perpAxis);
			const auto secondInterval = projectPolygonOnAxis(triangleTwo, perpAxis);

			canDrawLineBetween = intervalDistance(firstInterval, secondInterval) > 0;
		}
		
		// for second triangle
		if (!canDrawLineBetween)
		{
			for (uint32_t index = 0, nextIndex = 1; index < triangleTwo.size(); ++index, ++nextIndex)
			{
				if (canDrawLineBetween)
					break;

				if (nextIndex == triangleTwo.size())
					nextIndex = 0;

				const glm::vec3 edge = triangleTwo[index] - triangleTwo[nextIndex];

				const glm::vec3 vectorUp(0.0, 1.0, 0.0);
				const auto perpAxis = glm::normalize(glm::cross(edge, vectorUp));

				const auto firstInterval = projectPolygonOnAxis(triangleOne, perpAxis);
				const auto secondInterval = projectPolygonOnAxis(triangleTwo, perpAxis);

				canDrawLineBetween = intervalDistance(firstInterval, secondInterval) > 0;
			}
		}
		
		return !canDrawLineBetween;
	}

	/*
	*	Points must be odrered in this order in order to work:
	*   
	*	P1	P2
	*	P3	P4
	*	P5	...
	*
	*/
	static bool polygonPolygonCollision(const Points& polygonOnePoints, const Points& polygonTwoPoints)
	{
		auto findCentreAndRadiusOfPoints = [](const Points& points)
		{
			float maxX = std::numeric_limits<float>::min();
			float minX = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();
			float minZ = std::numeric_limits<float>::max();

			Point average = {};
			for (const auto& p : points)
			{
				average += p;
				maxX = std::max(maxX, p.x);
				minX = std::min(minX, p.x);
				maxY = std::max(maxY, p.y);
				minY = std::min(minY, p.y);
				maxZ = std::max(maxZ, p.z);
				minZ = std::min(minZ, p.z);
			}
			average /= float(points.size());
			auto getGreater = [](float a, float b)
			{return a > b ? a : b; };

			float absX = getGreater(std::abs(minX - average.x), std::abs(maxX - average.x));
			float absY = getGreater(std::abs(minY - average.y), std::abs(maxY - average.y));
			float absZ = getGreater(std::abs(minZ - average.z), std::abs(maxZ - average.z));

			Point radiusPoint(absX, absY, absZ);

			return std::make_pair(average,glm::length(radiusPoint));
		};

		// can improve performance in several occasions idk tho,
		// premature optimalisation it is
		constexpr const uint32_t minPCountToVerify = 14;
		if (polygonOnePoints.size() + polygonTwoPoints.size() > minPCountToVerify)
		{
			const auto [c1, r1] = findCentreAndRadiusOfPoints(polygonOnePoints);
			const auto [c2, r2] = findCentreAndRadiusOfPoints(polygonTwoPoints);

			const float circleDistances = glm::length(c1 - c2);

			bool circlesIntersect = circleDistances <= std::abs(r1 + r2);

			if (!circlesIntersect)
				return false;
		}
		// +2 for triangle
		for (uint32_t firstIndex = 0; firstIndex + 2 < polygonOnePoints.size(); ++firstIndex)
		{
			const Triangle polygonOneTriangle =
			{ polygonOnePoints[firstIndex],  polygonOnePoints[firstIndex + 1] ,polygonOnePoints[firstIndex + 2] };

			// 2+ as well
			for (uint32_t secondIndex = 0; secondIndex + 2 < polygonTwoPoints.size(); ++secondIndex)
			{
				const Triangle polygonTwoTriangle =
				{ polygonTwoPoints[secondIndex],  polygonTwoPoints[secondIndex + 1] ,polygonTwoPoints[secondIndex + 2] };

				if (triangleTriangleCollision(polygonOneTriangle, polygonTwoTriangle))
					return true;
			}
		}

		return false;
	}
}