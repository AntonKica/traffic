#include "Collider2D.h"
#include "BasicGeometry.h"
#include "SimulationObject.h"
#include "GlobalObjects.h"

#include <utility>
#include <algorithm>
#include <boost/geometry.hpp>
#include <iostream>

namespace
{

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

/*
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
*/

void Collider2D::setBoundaries(const Points& newBoundaries)
{
	m_boundaries = newBoundaries;

	setupCircle();
	updateCollisionBoundaries();
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


bool Collider2D::canCollideWith(const Collider2D& other) const
{
	return canCollideWith(other.m_tags);
}

bool Collider2D::canCollideWith(uint32_t otherTag) const
{
	return m_otherTags & otherTag;
}

bool Collider2D::collidesWith(const Collider2D& other) const
{
	// check before doing any calculations
	if (Collider::circlesOverlay(m_collisionCircle, other.m_collisionCircle))
	{
		return Collision::polygonPolygon(m_collisionBoundaries, other.m_collisionBoundaries);
	}

	return false;
}

bool Collider2D::collidesWith(const Points& points) const
{
	return Collision::polygonPolygon(Collision::details::createXZPolygonFromPoints(points), m_collisionBoundaries);
}

bool Collider2D::collidesWith(const Point& point) const
{
	return Collision::pointPolygon(Collision::details::createPointXZFromPoint(point), m_collisionBoundaries);
}

void Collider2D::setSelfTags(const std::vector<std::string>& newSelfTags)
{
	m_tags = App::physics.getTagsFlag(newSelfTags);
}

void Collider2D::setOtherTags(const std::vector<std::string>& newOtherTags)
{
	m_otherTags = App::physics.getTagsFlag(newOtherTags);
}

void Collider2D::setTags(const std::vector<std::string>& newSelfTags, const std::vector<std::string>& newOtherTags)
{
	setSelfTags(newSelfTags);
	setOtherTags(newOtherTags);
}

void Collider2D::resetSelfTags()
{
	m_tags = 0;
}

void Collider2D::resetOtherTags()
{
	m_otherTags = 0;
}

void Collider2D::resetTags()
{
	m_tags = 0;
	m_otherTags = 0;
}

bool Collider2D::hasSelfTags() const
{
	return m_tags != 0;
}

bool Collider2D::hasOtherTags() const
{
	return m_otherTags != 0;
}

bool Collider2D::isInCollison() const
{
	return !m_currentlyInCollision.empty();
}

std::vector<SimulationObject*> Collider2D::getAllCollisionWith(std::string tagName) const
{
	const auto checkTag = App::physics.getTagFlag(tagName);

	std::set<SimulationObject*> collidedWith;
	for (const auto& [collider, object] : m_currentlyInCollision)
	{
		if (collider->m_tags & checkTag)
			collidedWith.emplace(object);
	}

	return std::vector(collidedWith.begin(), collidedWith.end());
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

void Collider2D::clearCollisions()
{
	m_currentlyInCollision.clear();
}

bool Collider2D::alreadyInCollisionWith(Collider2D* collider) const
{
	for (const auto& [collisionCollider, collisionObject] : m_currentlyInCollision)
	{
		if (collider == collisionCollider)
			return true;
	}

	return false;
}

void Collider2D::addCollision(Collider2D* collider, SimulationObject* collisionObject)
{
	Collider2DSimulationObjectPair pair = { collider, collisionObject };
	m_currentlyInCollision.push_back(pair);
}
