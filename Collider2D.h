#pragma once
#include "BasicGeometry.h"

namespace Collider
{
	struct Circle
	{
		Point centre = {};
		float radius = {};
	};
	static Circle createCircle(const Points& points);
	bool circlesOverlay(const Circle& firstCircle, const Circle& secondCircle);
}

class Collider2D
{
	friend class PhysicsComponent;
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
private:
	Collider2D();

	void setupTriangles();
	void setupCircle();

	void updateCollisionCircle();
	void updateCollisionTriangles();

	Points m_boundaries;

	Collider::Circle m_circle;
	Triangles m_triangles;
	glm::vec3 m_position;
	glm::vec3 m_rotation;

	Collider::Circle m_collisionCircle;
	Triangles m_collisionTriangles;
};

