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
public:
	Collider2D();
	Collider2D(Points boundaries);
	Collider2D(Points boundaries, glm::vec3 position);
	Collider2D(Points boundaries, glm::vec3 position, glm::vec3 rotation);

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);

	bool collides(const Collider2D& other) const;
private:
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

