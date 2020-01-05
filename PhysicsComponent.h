#pragma once

// for now
#include "Collider2D.h"

class PhysicsComponent;
using pPhysicsComponent = PhysicsComponent*;

class PhysicsComponent
{
public:
	friend class Physics;
	static pPhysicsComponent const createPhysicsComponent();
	static pPhysicsComponent const copyPhysicsComponent(const pPhysicsComponent& const copyPhysicsComponent);
	static void destroyPhysicsComponent(pPhysicsComponent& physicsComponent);


	void setActive(bool active);
	bool active() const;

	bool inCollision() const;

	Collider2D& collider();
	const Collider2D& collider() const;
private:
	PhysicsComponent();

	bool m_active;
	bool m_inCollision;
	Collider2D collider2D;
};

