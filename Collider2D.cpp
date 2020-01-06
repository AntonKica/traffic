#include "Collider2D.h"
#include "BasicGeometry.h"

#include <utility>
#include <algorithm>

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

void Collider2D::set(const Points& newBoundaries, const glm::vec3& newPosition, const glm::vec3& newRotation)
{
	m_boundaries = newBoundaries;
	m_position = newPosition;
	m_rotation = newRotation;

	setupCircle();
	setupTriangles();
}

void Collider2D::set(const glm::vec3& newPosition, const glm::vec3& newRotation)
{
	m_position = newPosition;
	m_rotation = newRotation;

	updateCollisionCircle();
	updateCollisionTriangles();
}

void Collider2D::setBoundaries(const Points& newBoundaries)
{
	m_boundaries = newBoundaries;

	setupCircle();
	setupTriangles();
}

void Collider2D::setPosition(const glm::vec3& newPosition)
{
	if (m_position != newPosition)
	{
		m_position = newPosition;

		updateCollisionCircle();
		updateCollisionTriangles();
	}
}


void Collider2D::setRotation(const glm::vec3& newRotation)
{
	if (newRotation != newRotation)
	{
		m_rotation = newRotation;

		updateCollisionTriangles();
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
		// then with each triangle
		for (auto triangle : m_collisionTriangles)
		{
			for (auto otherTriangle : other.m_collisionTriangles)
			{
				if (triangleTriangleCollision(triangle, otherTriangle))
					return true;
			}
		}
	}

	return false;
}

void Collider2D::setupCircle()
{
	m_circle = Collider::createCircle(m_boundaries);

	updateCollisionCircle();
}

void Collider2D::setupTriangles()
{
	if (m_boundaries.size() >= 3)
		m_triangles = createTriangleFromPoints(m_boundaries);
	else
		m_triangles.clear();

	updateCollisionTriangles();
}

void Collider2D::updateCollisionCircle()
{
	m_collisionCircle = m_circle;
	m_collisionCircle.centre += m_position;
}

void Collider2D::updateCollisionTriangles()
{
	m_collisionTriangles.clear();

	std::transform(std::begin(m_triangles), std::end(m_triangles), std::back_inserter(m_collisionTriangles),
		[&](const Triangle& triangle)
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

			Triangle newTriangle;
			newTriangle[0] = rotatePointInXZPlane(m_circle.centre, m_rotation.x, triangle[0]) + m_position;
			newTriangle[1] = rotatePointInXZPlane(m_circle.centre, m_rotation.x, triangle[1]) + m_position;
			newTriangle[2] = rotatePointInXZPlane(m_circle.centre, m_rotation.x, triangle[2]) + m_position;

			return newTriangle;
		});
}
