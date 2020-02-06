#pragma once
#include "BasicGeometry.h"
#include "Collisions.h"

namespace Collider
{
	struct Circle
	{
		Point centre = {};
		float radius = {};
	};
	static Circle createCircle(const Points& points);
	bool circlesOverlay(const Circle& firstCircle, const Circle& secondCircle);

	struct Rectangle
	{
		Point topLeft, topRight, bottomLeft, bottomRight;
	};
	static Rectangle createRectangle(const Points& points);
	bool rectanglesOverlay(const Rectangle& firstCircle, const Rectangle& secondCircle);
}

class Collider2D
{
	friend class Physics;
	friend class PhysicsComponent;
	friend class PhysicsComponentCore;
public:
	void set(const Points& boundaries, const glm::vec3& newPosition, const glm::vec3& newRotation);
	void set(const glm::vec3& newPosition, const glm::vec3& newRotation);
	void setBoundaries(const Points& boundaries);
	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);

	const Points& getBoundaries() const;
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;

	bool collides(const Collider2D& other) const;
	bool collides(const Points& point) const;
	bool collides(const Point& point) const;
private:
	Collider2D() = default;

	void setupCircle();

	void updateCollisionCircle();
	void updateCollisionBoundaries();

	Points m_boundaries;

	Collider::Circle m_circle;
	glm::vec3 m_position = {};
	glm::vec3 m_rotation = {};

	Collider::Circle m_collisionCircle;
	CL::PolygonXZ m_collisionBoundaries;
};

