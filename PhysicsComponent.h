#pragma once

// for now
#include "PhysicsInfo.h"
#include "Collider2D.h"


struct PhysicsComponentCore
{
	Collider2D collider2D;

	uint32_t tag = 0;
	uint32_t otherTags = 0;

	uint32_t inCollisionWith = 0;
	bool active = false;
};
using pPhysicsComponentCore = PhysicsComponentCore*;

class PhysicsComponent
{
public:
	friend class Physics;

	PhysicsComponent();
	~PhysicsComponent();
	PhysicsComponent(const PhysicsComponent& copy);
	PhysicsComponent(PhysicsComponent&& move) noexcept;
	PhysicsComponent& operator=(const PhysicsComponent& copy);
	PhysicsComponent& operator=(PhysicsComponent&& move) noexcept;

	void updateSelfCollisionTags(const std::vector<std::string>& newSelfTags);
	void updateOtherCollisionTags(const std::vector<std::string>& newOtherTags);
	void updateCollisionTags(const Info::PhysicsComponentUpdateTags& updateInfo);

	void setActive(bool active);
	bool active() const;

	bool inCollision() const;
	bool inCollision(std::string tag) const;

	Collider2D& collider();
	const Collider2D& collider() const;

private:
	pPhysicsComponentCore m_core = nullptr;
};

