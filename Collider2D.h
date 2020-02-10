#pragma once
#include "BasicGeometry.h"
#include "Collisions.h"
#include <map>
#include <string>

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

class SimulationObject;
class Collider2D
{
	friend class Physics;
	friend class PhysicsComponent;
	friend class PhysicsComponentCore;
public:
	void setBoundaries(const Points& boundaries);

	const Points& getBoundaries() const;
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;

	bool canCollideWith(const Collider2D& other) const;
	bool canCollideWith(uint32_t otherTag) const;
	bool collidesWith(const Collider2D& other) const;
	bool collidesWith(const Points& point) const;
	bool collidesWith(const Point& point) const;

	void setSelfTags(const std::vector<std::string>& newSelfTags);
	void setOtherTags(const std::vector<std::string>& newOtherTags);
	void setTags(const std::vector<std::string>& newSelfTags, const std::vector<std::string>& newOtherTags);

	void resetSelfTags();
	void resetOtherTags();
	void resetTags();

	bool hasSelfTags() const;
	bool hasOtherTags() const;
	bool isInCollison() const;
	std::vector<SimulationObject*> getAllCollisionWith(std::string tagName) const;
private:
	//Collider2D() = default;

	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);
	void setupCircle();

	void updateCollisionCircle();
	void updateCollisionBoundaries();

	void clearCollisions();
	bool alreadyInCollisionWith(Collider2D* collider) const;
	void addCollision(Collider2D* collider, SimulationObject* collisionObject);

	uint32_t m_tags = 0;
	uint32_t m_otherTags = 0;
	struct Collider2DSimulationObjectPair
	{
		Collider2D* collider = nullptr;
		SimulationObject* object = nullptr;
	};
	std::vector<Collider2DSimulationObjectPair> m_currentlyInCollision;

	Points m_boundaries;

	Collider::Circle m_circle;
	glm::vec3 m_position = {};
	glm::vec3 m_rotation = {};

	Collider::Circle m_collisionCircle;
	CL::PolygonXZ m_collisionBoundaries;
};

