#pragma once
#include "BasicGeometry.h"

#include <utility>
#include <optional>
#include <stdexcept>
#include <algorithm>

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
		NonCopy(NonCopy&&) = delete;
		NonCopy& operator= (const NonCopy&) = delete;
		NonCopy& operator= (NonCopy&&) = delete;
	};

	class PrivateClass
	{
	protected:
		PrivateClass() = default;
		PrivateClass(const PrivateClass&) = default;
		PrivateClass(PrivateClass&&) = default;
		PrivateClass& operator= (const PrivateClass&) = default;
		PrivateClass& operator= (PrivateClass&&) = default;
	};

	template<class T>
	size_t generateNextContainerID(T c)
	{
		return c.size();
	}
}

static Point vectorIntersection(Point s1, Point e1, Point s2, Point e2);
static bool pointSitsOnLine(const Line& line, const Point& point);
template<class PointType1, class PointType2, class PointType3> bool pointSitsOnLine(const PointType1& s, const PointType2& e, const PointType3& p);

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

static bool polygonPointCollision(const Points& polygonPoints, Point point)
{
	bool collision = false;
	for (auto vert = polygonPoints.begin(); vert != polygonPoints.end(); ++vert)
	{
		auto nextVert = (vert + 1) ==
			polygonPoints.end() ? polygonPoints.begin() : vert + 1;

		// z test
		if (((vert->z > point.z) && nextVert->z < point.z)
			|| (vert->z < point.z && (nextVert->z > point.z)))
		{
			if (point.x < (nextVert->x - vert->x) * (point.z - vert->z) / (nextVert->z - vert->z) + vert->x)
			{
				collision = !collision;
			}
		}
	}

	return collision;
}

/*
* -1 = CCW
*  0 = COLINEAR
*  1 = CW
*/
static int triangleOrientation(const Triangle& triangle)
{
	auto value = (triangle[1].z - triangle[0].z) * (triangle[2].x - triangle[1].x) -
				(triangle[1].x - triangle[0].x) * (triangle[2].z - triangle[1].z);

	if(glm::epsilonEqual(value, 0.0f, 0.001f))
		return 0;
	if (value > 0.0f)
		return 1;
	else //if (slopeOne < slopeTwo)
		return -1;
}

// Given three colinear points p1, p2, p3, the function checks if 
// point p3 lies on line segment 'pr' 
static bool onSegment(Point p1, Point p2, Point p3)
{
	return p3.x <= std::max(p1.x, p2.x) && p3.x >= std::min(p1.x, p2.x) &&
		p3.z <= std::max(p1.z, p2.z) && p3.z >= std::min(p1.z, p2.z);
}

static bool lineLineCollision(const Line& lineOne, const Line& lineTwo)
{
	/*
	auto orientationOne = triangleOrientation(Triangle{ lineOne[0], lineTwo[0], lineOne[1] });
	auto orientationTwo = triangleOrientation(Triangle{ lineOne[0], lineTwo[0], lineTwo[1] });
	auto orientationThree = triangleOrientation(Triangle{ lineOne[1], lineTwo[1], lineOne[0] });
	auto orientationFour = triangleOrientation(Triangle{ lineOne[1], lineTwo[1], lineTwo[0] });

	if (orientationOne != orientationTwo &&
		orientationThree != orientationFour)
	{
		return true;
	}
	else if (orientationOne == 0 && onSegment(lineOne[0], lineOne[1], lineTwo[0]))
	{
		return true;
	}
	else if (orientationTwo == 0 && onSegment(lineOne[0], lineTwo[1], lineTwo[0]))
	{
		return true;
	}
	else if (orientationThree == 0 && onSegment(lineOne[1], lineOne[0], lineTwo[1]))
	{
		return true;
	}
	else if (orientationFour == 0 && onSegment(lineOne[1], lineTwo[0], lineTwo[1]))
	{
		return true;
	}
	else
	{
		return false;
	}
	*/
	auto intersectionPoint = vectorIntersection(lineOne[0], lineOne[1], lineTwo[0], lineTwo[1]);

	return pointSitsOnLine(lineOne, intersectionPoint) && pointSitsOnLine(lineTwo, intersectionPoint);
}
static bool innerLineLineCollision(const Line& lineOne, const Line& lineTwo)
{
	if (lineLineCollision(lineOne, lineTwo))
	{
		auto intersectionPoint = vectorIntersection(lineOne[0], lineOne[1], lineTwo[0], lineTwo[1]);
		return !(approxSamePoints(intersectionPoint, lineOne[0]) ||
			approxSamePoints(intersectionPoint, lineOne[1]) ||
			approxSamePoints(intersectionPoint, lineTwo[0]) ||
			approxSamePoints(intersectionPoint, lineTwo[1]));
	}
	else
	{
		return false;
	}
}


using Trail = Points;
static bool trailTrailCollision(const Trail& trailOne, const Trail& trailTwo)
{
	if (trailOne.size() < 2 || trailTwo.size() < 2)
		return false;

	for (uint32_t indexOne = 0; indexOne < trailOne.size() - 1; ++indexOne)
	{
		for (uint32_t indexTwo = 0; indexTwo < trailTwo.size() - 1; ++indexTwo)
		{
			Line one = { trailOne[indexOne], trailOne[indexOne + 1] };
			Line two = { trailTwo[indexTwo], trailTwo[indexTwo + 1] };
			if (lineLineCollision(one, two))
				return true;
		}
	}

	return false;
}

/*
* returns if cut was successful
*/
static std::optional<std::pair<Trail, Trail>> cutTwoTrailsOnCollision(Trail& trailOne, Trail& trailTwo)
{
	std::optional<std::pair<Trail, Trail>> optionalCut;

	if (trailOne.size() < 2 || trailTwo.size() < 2)
	{
		for (auto oneIt = trailOne.begin(); oneIt + 1 < trailOne.end() && !optionalCut; ++oneIt)
		{
			for (auto twoIt = trailTwo.begin(); twoIt + 1 < trailTwo.end() && !optionalCut; ++twoIt)
			{
				Line one = { *oneIt, *(oneIt + 1) };
				Line two = { *twoIt, *(twoIt + 1) };
				if (lineLineCollision(one, two))
				{
					// get that point
					Point intersectionPoint = vectorIntersection(one[0], one[1], two[0], two[1]);
					// add then insert before each line ed
					auto insertOne = trailOne.insert(oneIt + 1, intersectionPoint);
					auto insertTwo = trailTwo.insert(twoIt + 1, intersectionPoint);
					
					// construct new
					Trail newTrailOne(insertOne, trailOne.end());
					Trail newTrailTwo(insertTwo, trailTwo.end());
					
					// remove from previous, withouyt inserted
					trailOne.erase(insertOne + 1, trailOne.end());
					trailTwo.erase(insertTwo + 1, trailTwo.end());
					
					optionalCut = std::make_pair(newTrailOne, newTrailTwo);
				}
			}
		}
	}

	return optionalCut;
}

static Point getTrailEndsIntersectionPoint(Trail& trailOne, Trail& trailTwo)
{
	if (trailOne.size() < 2 || trailTwo.size() < 2)
		throw std::runtime_error("Cannot intersect trails which consist of one point!");

	Line lineOne = { *(trailOne.end() - 2), *(trailOne.end() - 1) };
	Line lineTwo = { *(trailTwo.end() - 2), *(trailTwo.end() - 1) };

	return vectorIntersection(lineOne[0], lineOne[1], lineTwo[0], lineTwo[1]);
}

template <class Type> 
struct MinMax
{
	Type min, max;
};

template <class PolygonType> MinMax<float> genericProjectPolygonOnAxis(const PolygonType& polygonPoints, const glm::vec3& axis)
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

template <class PolygonType> bool genericPolygonPolygonCollision(const PolygonType& polygonOne, const PolygonType& polygonTwo)
{
	bool canDrawLineBetween = false;

	// with first triangle
	for (uint32_t index = 0, nextIndex = 1; index < polygonOne.size(); ++index, ++nextIndex)
	{
		if (canDrawLineBetween)
			break;

		if (nextIndex == polygonOne.size())
			nextIndex = 0;

		const glm::vec3 edge = polygonOne[index] - polygonOne[nextIndex];

		const glm::vec3 vectorUp(0.0, 1.0, 0.0);
		const auto perpAxis = glm::normalize(glm::cross(edge, vectorUp));

		const auto firstInterval = genericProjectPolygonOnAxis(polygonOne, perpAxis);
		const auto secondInterval = genericProjectPolygonOnAxis(polygonTwo, perpAxis);

		canDrawLineBetween = intervalDistance(firstInterval, secondInterval) > 0;
	}

	// for second triangle
	if (!canDrawLineBetween)
	{
		for (uint32_t index = 0, nextIndex = 1; index < polygonTwo.size(); ++index, ++nextIndex)
		{
			if (canDrawLineBetween)
				break;

			if (nextIndex == polygonTwo.size())
				nextIndex = 0;

			const glm::vec3 edge = polygonTwo[index] - polygonTwo[nextIndex];

			const glm::vec3 vectorUp(0.0, 1.0, 0.0);
			const auto perpAxis = glm::normalize(glm::cross(edge, vectorUp));

			const auto firstInterval = genericProjectPolygonOnAxis(polygonOne, perpAxis);
			const auto secondInterval = genericProjectPolygonOnAxis(polygonTwo, perpAxis);

			canDrawLineBetween = intervalDistance(firstInterval, secondInterval) > 0;
		}
	}

	return !canDrawLineBetween;
}


static bool polygonsCollide(const Points& polygonOne, const Points& polygonTwo)
{
	if (polygonOne.size() <= 2 || polygonOne.size() <= 2)
		return false;

	for (uint32_t indexOne = 0; indexOne + 2 < polygonOne.size();++indexOne)
	{
		Triangle triangleOne = { polygonOne[indexOne], polygonOne[indexOne + 1], polygonOne[indexOne + 2] };
		for (uint32_t indexTwo = 0; indexTwo + 2 < polygonTwo.size(); ++indexTwo)
		{
			Triangle triangleTwo = { polygonTwo[indexTwo], polygonTwo[indexTwo + 1], polygonTwo[indexTwo + 2] };
			if (genericPolygonPolygonCollision(triangleOne, triangleTwo))
				return true;
		}
	}

	return false;
}

bool pointSitsOnLine(const Line& line, const Point& point)
{
	return pointSitsOnLine(line[0], line[1], point);
}

template<class PointType1, class PointType2, class PointType3> bool pointSitsOnLine(const PointType1& s, const PointType2& e, const PointType3& p)
{
	if (p == s || p == e)
		return true;

	glm::dvec3 ds(s);
	glm::dvec3 de(e);
	glm::dvec3 dp(p);

	constexpr const double maxDifference = 0.000'001;
	auto lengthSP = glm::length(ds - dp);
	auto lengthPE = glm::length(dp - de);
	auto lengthSE = glm::length(ds - de);

	return glm::epsilonEqual(lengthSP + lengthPE, lengthSE, maxDifference);
}


/* 
*	0 = Closer to trail's start
*	1 = in the middle
*	2 = Closer to trail's end
*/
using Trail = Points;
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
*	returns travelled trail  (including start point) and remaining distance 
*	if hit the end before travelling
*	also, if the trail is circle, it doesn't care
*/ 
template <class TrailContainer, class TrailPointType> std::pair<TrailContainer, std::optional<float>> travellDistanceOnTrailFromPointInDirection(
	float distanceToTravell,
	const TrailContainer& trail,
	TrailPointType trailPoint, 
	bool reverseDirection)
{
	TrailContainer travelledPoints = { trailPoint };
	float distanceLeft = distanceToTravell;

	if (!reverseDirection)
	{
		auto pointIt = std::find(std::begin(trail), std::end(trail), trailPoint);
		if (pointIt == std::end(trail))
			throw std::runtime_error("Given wrong trail point!");

		// befor doing any further calculations, check if is not already at the end
		if( !(*pointIt == trail.back() && trail.front() != trail.back()))
		{
			float travelledDistance = 0;
			for (auto previousIter = pointIt++; pointIt != std::end(trail); previousIter = pointIt++)
			{
				float thisLineDistance = glm::length(*pointIt - *previousIter);
				travelledDistance += thisLineDistance;

				if (travelledDistance > distanceToTravell)
				{
					auto dir = glm::normalize(*pointIt - *previousIter);
					float overStep = travelledDistance - distanceToTravell;
					float distanceFromPoint = thisLineDistance - overStep;

					auto finalPoint = *previousIter + dir * distanceFromPoint;

					travelledPoints.emplace_back(finalPoint);
					distanceLeft = 0;

					break;
				}
				else
				{
					travelledPoints.emplace_back(*pointIt);
					distanceLeft -= thisLineDistance;
				}
			}
		}
	}
	else
	{
		auto pointIt = std::find(std::rbegin(trail), std::rend(trail), trailPoint);
		if (pointIt == std::rend(trail))
			throw std::runtime_error("Given wrong trail point!");

		// befor doing any further calculations, check if is not already at the end
		if (!(*pointIt == trail.front() && trail.front() != trail.back()))
		{
			float travelledDistance = 0;
			for (auto previousIter = pointIt++; pointIt != std::rend(trail); previousIter = pointIt++)
			{
				float thisLineDistance = glm::length(*pointIt - *previousIter);
				travelledDistance += thisLineDistance;

				if (travelledDistance > distanceToTravell)
				{
					auto dir = glm::normalize(*pointIt - *previousIter);
					float overStep = travelledDistance - distanceToTravell;
					float distanceFromPoint = thisLineDistance - overStep;

					auto finalPoint = *previousIter + dir * distanceFromPoint;

					travelledPoints.emplace_back(finalPoint);
					distanceLeft = 0;

					break;
				}
				else
				{
					travelledPoints.emplace_back(*pointIt);
					distanceLeft -= thisLineDistance;
				}
			}
		}
	}
	std::optional<float> remainingDistance;
	if (distanceLeft)
		remainingDistance = distanceLeft;
	
	return std::make_pair(travelledPoints, remainingDistance);
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

template<class Type> Type getClosestPointToLine(
	const Type& lineStart,
	const Type& lineEnd,
	const Type& point)
{
	if (approxSamePoints(lineStart, lineEnd)) return lineStart;
	if (pointSitsOnLine(lineStart, lineEnd, point)) return point;

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

template<class C, class T> auto insertElemementBetween(C& container, T first, T second, T element)
{
	for (auto begin = std::begin(container); begin + 1 != std::end(container); ++begin)
	{
		// keep order of elements
		if (*begin == first && *(begin + 1) == second)
			return container.insert(begin + 1, element);
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

	Point left = vectorIntersection(p1 - previousRightVec, p2 - previousRightVec,
		p2 - currentRightVec, p3 - currentRightVec);

	Point right = vectorIntersection(p1 + previousRightVec, p2 + previousRightVec,
		p2 + currentRightVec, p3 + currentRightVec);

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