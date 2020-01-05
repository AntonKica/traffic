#include "PhysicsComponent.h"
#include "GlobalObjects.h"

PhysicsComponent* const PhysicsComponent::createPhysicsComponent()
{
	return App::physics.createPhysicsComponent();
}

pPhysicsComponent const PhysicsComponent::copyPhysicsComponent(const pPhysicsComponent& const copyPhysicsComponent)
{
	return App::physics.copyPhysicsComponent(copyPhysicsComponent);
}

void PhysicsComponent::destroyPhysicsComponent(pPhysicsComponent& physicsComponent)
{
	App::physics.destroyPhysicsComponent(physicsComponent);
}

void PhysicsComponent::setActive(bool active)
{
	m_active = active;
}

bool PhysicsComponent::active() const
{
	return m_active;
}

bool PhysicsComponent::inCollision() const
{
	return m_inCollision;
}

Collider2D& PhysicsComponent::collider()
{
	return collider2D;
}

const Collider2D& PhysicsComponent::collider() const
{
	return collider2D;
}

PhysicsComponent::PhysicsComponent()
	: m_active(false)
{
}
