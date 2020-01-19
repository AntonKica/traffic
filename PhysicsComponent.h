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

	std::unordered_map<uint32_t, SimulationObject*> inCollisionWith;

	//
	SimulationObject* pOwner = nullptr;
	bool active = false;
};
using pPhysicsComponentCore = PhysicsComponentCore*;

class PhysicsComponent
{
public:
	void updateSelfCollisionTags(const std::vector<std::string>& newSelfTags);
	void updateOtherCollisionTags(const std::vector<std::string>& newOtherTags);
	void updateCollisionTags(const Info::PhysicsComponentUpdateTags& updateInfo);

	void setActive(bool active);
	bool isActive() const;

	bool inCollision() const;
	SimulationObject* inCollision(std::string tag) const;

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

	SimulationObject* m_pOwner = nullptr;
	pPhysicsComponentCore m_core = nullptr;
};

