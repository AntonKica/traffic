#pragma once
#include <utility>

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

inline std::pair<Point, Point> findTwoClosestPoints(const Points& points, const Point& point)
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

template<class C, class T1, class T2, class T3> int insertElemementBetween(C& container, T1 first, T2 second, T3 element)
{
	for (int pointIndex = 0; pointIndex + 1 < container.size(); ++pointIndex)
	{
		if ((container[pointIndex] == first && container[pointIndex + 1] == second) ||
			(container[pointIndex + 1] == first && container[pointIndex] == second))
		{
			container.insert(std::begin(container) + pointIndex + 1, element);
			container.erase(std::unique(std::begin(container), std::end(container)), std::end(container));
			return pointIndex + 1;
		}
	}


	std::throw_with_nested("Supplied range, which doesnt exist in supplied container!");
}


inline Point vectorIntersection(Point s1, Point e1, Point s2, Point e2)
{
	float a1 = e1.z - s1.z;
	float b1 = s1.x - e1.x;
	float c1 = a1 * s1.x + b1 * s1.z;

	float a2 = e2.z - s2.z;
	float b2 = s2.x - e2.x;
	float c2 = a2 * s2.x + b2 * s2.z;

	float delta = a1 * b2 - a2 * b1;

	return delta == 0 ? s2 : Point((b2 * c1 - b1 * c2) / delta, 0.0f, (a1 * c2 - a2 * c1) / delta);
}
inline std::pair<Point, Point> getSidePoints(glm::vec3 firstDirection, glm::vec3 secondDirection, Point p1, Point p2, Point p3, float width)
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