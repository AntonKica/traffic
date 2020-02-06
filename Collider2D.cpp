#include "Collider2D.h"
#include "BasicGeometry.h"

#include <utility>
#include <algorithm>
#include <boost/geometry.hpp>
#include <iostream>

namespace
{
	bool polygonPointCollision(const Points& polygonPoints, Point point)
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

	template <class T> struct MinMax
	{
		T min, max;
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

	float intervalDistance(const MinMax<float>& firstInterval, const MinMax<float>& secondInterval)
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

	constexpr auto triangleTriangleCollision = genericPolygonPolygonCollision<Triangle>;
	constexpr auto quadrangleQuadrangleCollision = genericPolygonPolygonCollision<Quadrangle>;

	Triangles createTriangleFromPoints(const Points& points)
	{
		const uint32_t triangleCount = points.size() - 2;
		Triangles triangles;
		triangles.resize(triangleCount);

		auto trianglesIt = triangles.begin();

		// create triangles
		auto firstPointIter = points.begin();
		auto secondPointIter = points.begin() + 1;
		auto thirdPointIter = points.begin() + 2;
		while (true)
		{
			if (thirdPointIter == points.end())
				break;

			Triangle newTriangle = { *firstPointIter++, *secondPointIter++, *thirdPointIter++ };
			*trianglesIt++ = newTriangle;
		}

		return triangles;
	}
}

struct MinMaxVectors
{
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());
};

MinMaxVectors getMinMaxFromPoints(const Points& points)
{
	MinMaxVectors minMax = {};

	for (const auto& p : points)
	{
		minMax.max.x = std::max(minMax.max.x, p.x);
		minMax.max.y = std::max(minMax.max.y, p.y);
		minMax.max.z = std::max(minMax.max.z, p.z);

		minMax.min.x = std::min(minMax.min.x, p.x);
		minMax.min.y = std::min(minMax.min.y, p.y);
		minMax.min.z = std::min(minMax.min.z, p.z);
	}

	return minMax;
}

Collider::Circle Collider::createCircle(const Points& points)
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

	Circle oc;
	oc.centre = average;
	oc.radius = glm::length(radiusPoint);

	return oc;
}

bool Collider::circlesOverlay(const Circle& firstCircle, const Circle& secondCircle)
{
	const auto& [c1, r1] = firstCircle;
	const auto& [c2, r2] = secondCircle;

	const float circleDistances = glm::length(c1 - c2);

	return circleDistances <= std::abs(r1 + r2);
}

Collider::Rectangle Collider::createRectangle(const Points& points)
{
	return Rectangle();
}

bool Collider::rectanglesOverlay(const Rectangle& firstCircle, const Rectangle& secondCircle)
{
	return false;
}

void Collider2D::set(const Points& newBoundaries, const glm::vec3& newPosition, const glm::vec3& newRotation)
{
	m_boundaries = newBoundaries;
	m_position = newPosition;
	m_rotation = newRotation;

	setupCircle();
	updateCollisionBoundaries();
}

void Collider2D::set(const glm::vec3& newPosition, const glm::vec3& newRotation)
{
	m_position = newPosition;
	m_rotation = newRotation;

	updateCollisionCircle();
	updateCollisionBoundaries();
}

void Collider2D::setBoundaries(const Points& newBoundaries)
{
	m_boundaries = newBoundaries;

	setupCircle();
	updateCollisionBoundaries();
}

void Collider2D::setPosition(const glm::vec3& newPosition)
{
	if (m_position != newPosition)
	{
		m_position = newPosition;

		updateCollisionCircle();
		updateCollisionBoundaries();
	}
}


void Collider2D::setRotation(const glm::vec3& newRotation)
{
	if (newRotation != newRotation)
	{
		m_rotation = newRotation;

		updateCollisionBoundaries();
	}
}

const Points& Collider2D::getBoundaries() const
{
	return m_boundaries;
}

glm::vec3 Collider2D::getPosition() const
{
	return m_position;
}

glm::vec3 Collider2D::getRotation() const
{
	return m_rotation;
}


bool Collider2D::collides(const Collider2D& other) const
{
	// check before doing any calculations
	if (Collider::circlesOverlay(m_collisionCircle, other.m_collisionCircle))
	{
		return Collision::polygonPolygon(m_collisionBoundaries, other.m_collisionBoundaries);
	}

	return false;
}

bool Collider2D::collides(const Points& points) const
{
	return Collision::polygonPolygon(Collision::details::createXZPolygonFromPoints(points), m_collisionBoundaries);
}

bool Collider2D::collides(const Point& point) const
{
	return Collision::pointPolygon(Collision::details::createPointXZFromPoint(point), m_collisionBoundaries);
}


void Collider2D::setupCircle()
{
	m_circle = Collider::createCircle(m_boundaries);

	updateCollisionCircle();
}

void Collider2D::updateCollisionCircle()
{
	m_collisionCircle = m_circle;
	m_collisionCircle.centre += m_position;
}

void Collider2D::updateCollisionBoundaries()
{
	std::vector<Point> transformedPoints(m_boundaries.size());
	std::transform(std::begin(m_boundaries), std::end(m_boundaries), std::begin(transformedPoints),
		[&](const Point& point)
		{
			auto rotatePointInXZPlane = [](glm::vec3 centre, float angle, Point point)
			{
				const float angleSin = glm::sin(angle);
				const float angleCos = glm::cos(angle);

				point -= centre;
				point.x = point.x * angleCos - point.z * angleSin;
				point.z = point.x * angleSin + point.z * angleCos;

				return point + centre;
			};

			Point newPoint = rotatePointInXZPlane(m_circle.centre, m_rotation.x, point) + m_position;
			return newPoint;
		});
	m_collisionBoundaries = Collision::details::createXZPolygonFromPoints(transformedPoints);
}
