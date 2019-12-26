#pragma once
#include <utility>
#include <vector>
#include <optional>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>

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
using Line = std::array<Point, 2>;
using Points = std::vector<Point>;
using Trail = Points;

static bool approxSamePoints(const Point& p1, const Point& p2)
{
	constexpr const float maxDiff = 0.01f;

	return glm::length(p1 - p2) <= maxDiff;
}

static float linePointAngle(Point s1, Point e1, Point p)
{
	return ((e1.x - s1.x) * (p.z - s1.z) - (e1.z - s1.z) * (p.x - s1.x));
}

static bool pointsSitsOnSameHalfOfPlane(Point s1, Point e1, Point p1, Point p2)
{
	return (linePointAngle(s1, e1, p1) >= 0) == (linePointAngle(s1, e1, p2) >= 0);
}

static bool pointSitsOnLine(Point s, Point e, Point p)
{
	if (p == s || p == e)
		return true;

	constexpr const float maxDifference = 0.001f;
	auto lengthSP = glm::length(s - p);
	auto lengthPE = glm::length(p - e);
	auto lengthSE = glm::length(s - e);

	return glm::epsilonEqual(lengthSP + lengthPE, lengthSE, maxDifference);
}


/* 
*	0 = Closer to trail's start
*	1 = in the middle
*	2 = Closer to trail's end
*/
enum class TrailPosiition
{
	CLOER_TO_START, 
	IN_THE_MIDDLE,
	CLOSER_TO_END
};
static TrailPosiition positionOfPointRelativeToTrailEnds(const Trail& trail, Point point)
{
	// precaution
	if (approxSamePoints(point, trail.front()))
		return TrailPosiition::CLOER_TO_START;
	else if(approxSamePoints(point, trail.back()))
		return TrailPosiition::CLOSER_TO_END;

	float distanceFromStartToPoint = 0;
	float totalDistance = 0;

	for (uint32_t index = 0; index + 1 < trail.size(); ++index)
	{
		const auto& cur1 = trail[index];
		const auto& cur2 = trail[index + 1];

		totalDistance += glm::length(cur1 - cur2);
		if (distanceFromStartToPoint == 0 && pointSitsOnLine(cur1, cur2, point))
		{
			float excessDistance = glm::length(point - cur2);
			distanceFromStartToPoint = totalDistance - excessDistance;
		}
	}

	float distanceFromPointToEnd = totalDistance - distanceFromStartToPoint;
	
	if (glm::epsilonEqual(distanceFromPointToEnd, distanceFromStartToPoint, glm::epsilon<float>()))
		return TrailPosiition::IN_THE_MIDDLE;
	else if (distanceFromStartToPoint < distanceFromPointToEnd)
		return TrailPosiition::CLOER_TO_START;
	else
		return TrailPosiition::CLOSER_TO_END;
}

/*	
*	Travells distance along trail
*	returns also remaining distance if hit the end
*	also, if the trail is circle, it doesn't care
*/
template <class TrailType, class TrailPointType> std::pair<TrailPointType, std::optional<float>> 
	travellDistanceFromPointOnTrailToEnd(const TrailType& trail, TrailPointType trailPoint, float distance)
{
	auto pointIt = std::find(std::begin(trail), std::end(trail), trailPoint);
	if (pointIt == std::end(trail))
	{
		throw std::runtime_error("Given wrong trail point!");
	}
	if ((*pointIt == trail.back() && !std::signbit(distance) ||
		*pointIt == trail.front() && std::signbit(distance)) && 
		trail.front() != trail.back())
	{
		return std::make_pair(*pointIt, std::optional(std::abs(distance)));
	}

	TrailPointType finalPoint = {};
	float travelledDistance = 0.0f;
	if (distance > 0)
	{
		finalPoint = trail.front();
		auto oneBeforePointIt = pointIt++;
		while (true)
		{
			travelledDistance += glm::length(*oneBeforePointIt - *pointIt);

			if (travelledDistance > distance)
			{
				travelledDistance = distance;

				glm::vec3 dir = glm::normalize(*oneBeforePointIt - *pointIt);
				float dst = glm::length(*oneBeforePointIt - *pointIt);
				// voila
				finalPoint = *pointIt + (dir * (dst - distance));
				break;
			}
			
			if (pointIt != std::begin(trail))
				oneBeforePointIt = pointIt--;
			else
				break;
		}
	}
	else
	{
		finalPoint = trail.back();
		// 
		distance = std::abs(distance);

		auto onePastPointIt = pointIt--;
		while (true)
		{
			travelledDistance += glm::length(*pointIt - *onePastPointIt);

			if (travelledDistance > distance)
			{
				travelledDistance = distance;

				glm::vec3 curDir = glm::normalize(*pointIt - *onePastPointIt);
				float curDst = glm::length(*pointIt - *onePastPointIt);
				// voila
				finalPoint = *onePastPointIt + (curDir * distance);
				break;
			}

			if (pointIt == std::begin(trail))
				break;
			else
				onePastPointIt = pointIt--;
		}
	}

	std::optional<float> remainingDistance;
		if (travelledDistance - distance)
			remainingDistance = travelledDistance - distance;
	
	return std::make_pair(finalPoint, remainingDistance);
}
template<class Container> void removeDuplicates(Container& container)
{
	for (auto begin = std::begin(container); begin != std::end(container); ++begin)
	{
		auto cursor = begin + 1;
		while (true)
		{
			cursor = std::find(cursor, std::end(container), *begin);
			if (cursor != std::end(container))
				cursor = container.erase(cursor);
			else
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

template<class PointType1, class PointType2, class PointType3> PointType1 getClosestPointToLine
(const PointType1& lineStart,
	const PointType2& lineEnd,
	const PointType3& point)
{
	if (approxSamePoints(lineStart, lineEnd)) return lineStart;

	glm::vec3 lineDirection = glm::normalize(lineEnd - lineStart);
	float lineLength = glm::length(lineEnd - lineStart);

	const glm::vec3 startToPointDir = glm::normalize(point - lineStart);
	float alpha = glm::acos(glm::dot(startToPointDir, lineDirection));
	float lineDistanceFromPointOne = glm::length(lineStart - point) * cos(alpha);

	return lineStart + lineDirection * lineDistanceFromPointOne;
}

template<class PointsType, class PointType> void setNewCirclePointsStart(PointsType& points, PointType point)
{
	removeDuplicates(points);
	auto pointIt = std::find(std::begin(points), std::end(points), point);// findJoint(m_joints, point);
	{
		int elementsToErase = pointIt - points.begin();
		points.insert(points.end(), points.begin(), pointIt);
		points.erase(points.begin(), points.begin() + elementsToErase);

		/// add same point to other end
		points.insert(points.end(), points.front());
	}
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
	const float minDegrees = 0.1f;
	const float minAngle = minDegrees * (glm::pi<float>() / 180.0f);
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