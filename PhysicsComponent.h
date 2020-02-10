#pragma once

// for now
#include "PhysicsInfo.h"
#include "Collider2D.h"
#include <string>
#include <unordered_map>

class SimulationObject;
struct PhysicsComponentCore
{
	SimulationObject* pOwner = nullptr;
	bool active = false;

	std::unordered_map<std::string, Collider2D> collider2Ds;

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::vec3& rotation);
};
using pPhysicsComponentCore = PhysicsComponentCore*;

class PhysicsComponent
{
public:
	Collider2D& createCollider(const std::string& name);

	Collider2D& getCollider(const std::string& name);
	const Collider2D& getCollider(const std::string& name) const;
private:
	friend class Physics;
	friend class SimulationObject;

	PhysicsComponent();
	~PhysicsComponent();
	PhysicsComponent(const PhysicsComponent& copy);
	PhysicsComponent(PhysicsComponent&& move) noexcept;
	PhysicsComponent& operator=(const PhysicsComponent& copy);
	PhysicsComponent& operator=(PhysicsComponent&& move) noexcept;

	void setOwner(SimulationObject* pNewOwner);

	void setPosition(glm::vec3 position);
	void setRotation(glm::vec3 rotation);
	void setActive(bool active);
	bool isActive() const;

	SimulationObject* m_pOwner = nullptr;
	pPhysicsComponentCore m_core = nullptr;
	glm::vec3 m_position = {};
	glm::vec3 m_rotation = {};
};

