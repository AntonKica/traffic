#pragma once

// for now
#include "PhysicsInfo.h"
#include "Collider2D.h"
#include <unordered_map>

class SimulationObject;
struct PhysicsComponentCore
{
	Collider2D collider2D;

	uint32_t tag = 0;
	uint32_t otherTags = 0;

	std::unordered_map<uint32_t, std::vector<SimulationObject*>> inCollisionWith;

	//
	SimulationObject* pOwner = nullptr;
	bool active = false;
};
using pPhysicsComponentCore = PhysicsComponentCore*;

class PhysicsComponent
{
public:
	void setSelfCollisionTags(const std::vector<std::string>& newSelfTags);
	void setOtherCollisionTags(const std::vector<std::string>& newOtherTags);
	void setCollisionTags(const Info::PhysicsComponentUpdateTags& updateInfo);

	bool inCollision() const;
	bool inCollisionWith(std::string tag) const;
	std::vector<SimulationObject*> getAllCollisionWith(std::string tag) const;

	Collider2D& collider();
	const Collider2D& collider() const;

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

	void setActive(bool active);
	bool isActive() const;

	SimulationObject* m_pOwner = nullptr;
	pPhysicsComponentCore m_core = nullptr;
};

