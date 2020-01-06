#pragma once

// for now
#include "Collider2D.h"


struct PhysicsComponentCore
{
	Collider2D collider2D;

	bool inCollision = false;
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

	void setActive(bool active);
	bool active() const;

	bool inCollision() const;

	Collider2D& collider();
	const Collider2D& collider() const;
private:
	pPhysicsComponentCore m_core = nullptr;
};

