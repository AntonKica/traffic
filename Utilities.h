#pragma once
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
namespace Utility
{
	class NonCopy
	{
	public:
		NonCopy() = default;
		NonCopy(const NonCopy&) = delete;
		NonCopy(const NonCopy&&) = delete;
		NonCopy& operator= (const NonCopy&) = delete;
		NonCopy& operator= (const NonCopy&&) = delete;
	};

	template<class T>
	size_t generateNextContainerID(T c)
	{
		return c.size();
	}
}

using Point = glm::vec3;
using Points = std::vector<Point>;

static bool approxSamePoints(const Point& p1, const Point& p2)
{
	constexpr const float maxDiff = 0.01f;

	return glm::length(p1 - p2) <= maxDiff;
}

template<class Container> void removeDuplicates(Container& container)
{
	for (auto begin = std::begin(container); begin != std::end(container); ++begin)
	{
		auto cursor = begin + 1;
		while (true)
		{
			cursor = container.erase(std::find(cursor, std::end(container), *begin), std::end(container));
			if (cursor == std::end(container))
				break;
		}
	}
}

static std::pair<Point, Point> findTwoClosestPoints(const Points& points, const Point& point)
{
	std::optional<Point> first, second;
	for (const auto& point_ : points)
	{
		if (!first)
		{
			first = point_;
			continue;
		}

		float currentDistance = glm::length(point - point_);
		float firstPointDistance = glm::length(point - first.value());
		if (currentDistance < firstPointDistance)
		{
			second = first;
			first = point_;
		}
		else if (!second)
		{
			second = point_;
		}
		else if (float secondPointDistance = glm::length(point - second.value());
			currentDistance < secondPointDistance)
		{
			second = point_;
		}
	}
	return std::make_pair(first.value(), second.value());
};

static Point findClosestPoint(const Points& points, const Point& point)
{
	std::optional<Point> closestPoint;
	for (const auto& point_ : points)
	{
		if (!closestPoint)
		{
			closestPoint = point_;
			continue;
		}

		float currentDistance = glm::length(point - point_);
		float closestDistance = glm::length(point - closestPoint.value());
		if (currentDistance < closestDistance)
			closestPoint = point_;
	}
	
	return closestPoint.value();
};

static std::pair<Point, Point> getClosestSideFromThreePoints(const Point& lineStart, const Point& lineJoint, const Point& lineEnd, const Point& point)
{
	const glm::vec3 dirToJointStart = glm::normalize(lineJoint - lineStart);
	const glm::vec3 dirToJointEnd = glm::normalize(lineJoint - lineEnd);
	const glm::vec3 dirToJointPoint = glm::normalize(lineJoint - point);

	const float startAngle = glm::degrees(glm::acos(glm::dot(dirToJointStart, dirToJointPoint)));
	const float endAngle = glm::degrees(glm::acos(glm::dot(dirToJointEnd, dirToJointPoint)));
	// acute angle
	std::pair<Point, Point> line;
	auto& [start, end] = line;
	if (startAngle <= 90.0f)
	{	
		// other obtuse angle
		if (endAngle > 90.0f || 
			startAngle < endAngle)
		{
			start = lineStart;
			end = lineJoint;
		}
		else
		{
			start = lineJoint;
			end = lineEnd;
		}
	}
	else
	{
		if (endAngle > 90.0f)
		{
			start = end = lineJoint;
		//	start = lineStart;
		//	end = lineJoint;
		}
		else
		{
			start = lineJoint;
			end = lineEnd;
		}
	}

	return line;
}

static Point getClosestPointToLine(const Point& lineStart, const Point& lineEnd, const Point& point)
{
	if (approxSamePoints(lineStart, lineEnd)) return lineStart;

	glm::vec3 lineDirection = glm::normalize(lineEnd - lineStart);
	float lineLength = glm::length(lineEnd - lineStart);

	const glm::vec3 startToPointDir = glm::normalize(point - lineStart);
	float alpha = glm::acos(glm::dot(startToPointDir, lineDirection));
	float lineDistanceFromPointOne = glm::length(lineStart - point) * cos(alpha);

	return lineStart + lineDirection * lineDistanceFromPointOne;
}

template<class C, class T1, class T2, class T3> auto insertElemementBetween(C& container, T1 first, T2 second, T3 element)
{
	for (auto begin = std::begin(container); begin != std::end(container) - 1; ++begin)
	{
		if ((*begin == first && *(begin + 1) == second) ||
			(*(begin + 1) == first && *begin == second))
		{
			return container.insert(begin + 1, element);
		}
	}

	throw std::runtime_error("Supplied range, which doesnt exist in supplied container!");
}


static Point vectorIntersection(Point s1, Point e1, Point s2, Point e2)
{
	float a1 = e1.z - s1.z;
	float b1 = s1.x - e1.x;
	float c1 = a1 * s1.x + b1 * s1.z;

	float a2 = e2.z - s2.z;
	float b2 = s2.x - e2.x;
	float c2 = a2 * s2.x + b2 * s2.z;

	// round if one degree
	const float minAngle = 1.0f * (glm::pi<float>() / 180.0f);
	float delta = a1 * b2 - a2 * b1;

	return std::abs(delta) <= minAngle ? s2 : Point((b2 * c1 - b1 * c2) / delta, 0.0f, (a1 * c2 - a2 * c1) / delta);
}
static std::pair<Point, Point> getSidePoints(glm::vec3 firstDirection, glm::vec3 secondDirection, Point p1, Point p2, Point p3, float width)
{
	const glm::vec3 vectorUp(0.0, 1.0, 0.0);

	glm::vec3 previousRightVec = glm::normalize(glm::cross(firstDirection, vectorUp)) * width / 2.0f;
	glm::vec3 currentRightVec = glm::normalize(glm::cross(secondDirection, vectorUp)) * width / 2.0f;

	Point left = vectorIntersection(p1 + previousRightVec, p2 + previousRightVec,
		p2 + currentRightVec, p3 + currentRightVec);

	Point right = vectorIntersection(p1 - previousRightVec, p2 - previousRightVec,
		p2 - currentRightVec, p3 - currentRightVec);

	return std::make_pair(left, right);
}

template<class T1, class T2> bool compareOptionals(std::optional<T1> lhs, std::optional<T2> rhs)
{
	if (lhs && rhs)			// both have types, then compare via value
		return lhs.value() == rhs.value();
	else if (!lhs && !rhs)	//neither have value, definitely equal
		return true;
	else					//homogenous, definitely not equal
		return false;
}