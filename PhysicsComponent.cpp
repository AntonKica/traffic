#include "PhysicsComponent.h"
#include "GlobalObjects.h"


Collider2D& PhysicsComponent::createCollider(const std::string& name)
{
	auto& newCollider = m_core->collider2Ds[name];

	// setup
	newCollider.setPosition(m_position);
	newCollider.setRotation(m_rotation);

	return newCollider;
}

Collider2D& PhysicsComponent::getCollider(const std::string& name)
{
	auto newCollider = m_core->collider2Ds.find(name);
	if (newCollider == m_core->collider2Ds.end())
		throw std::runtime_error("Trying to accses collider with name " + name + " that wasnt created previously!");

	return newCollider->second;
}

const Collider2D& PhysicsComponent::getCollider(const std::string& name) const 
{
	const auto newCollider = m_core->collider2Ds.find(name);
	if (newCollider == m_core->collider2Ds.end())
		throw std::runtime_error("Trying to accses collider with name " + name + " that wasnt created previously!");

	return newCollider->second;
}

PhysicsComponent::PhysicsComponent()
{
	m_core = App::physics.createPhysicsComponentCore();
	m_core->pOwner = m_pOwner;
}

PhysicsComponent::~PhysicsComponent()
{
	if (m_core)
		App::physics.deactivatePhysicsComponentCore(m_core);
}

PhysicsComponent::PhysicsComponent(const PhysicsComponent& copy)
{
	if (this == &copy)
		return;

	if (copy.m_core)
	{
		m_core->pOwner = m_pOwner;
		m_core = App::physics.copyCreatePhysicsComponentCore(copy.m_core);
	}

}

PhysicsComponent::PhysicsComponent(PhysicsComponent&& move) noexcept
{
	if (this == &move)
		return;

	m_core = move.m_core;	move.m_core = nullptr;
	if (m_core)
	{
		m_core->pOwner = m_pOwner;
	}
}

PhysicsComponent& PhysicsComponent::operator=(const PhysicsComponent& copy)
{
	if (this == &copy)
		return *this;

	if (m_core)
	{
		if (copy.m_core)
			App::physics.copyPhysicsComponentCore(copy.m_core, m_core);
		else
			App::physics.deactivatePhysicsComponentCore(m_core);
	}
	else if (copy.m_core)
	{
		if (copy.m_core)
			App::physics.copyCreatePhysicsComponentCore(copy.m_core);
	}

	if (m_core)
	{
		m_core->pOwner = m_pOwner;
	}

	return *this;
}

PhysicsComponent& PhysicsComponent::operator=(PhysicsComponent&& move) noexcept
{
	if (this == &move)
		return *this;

	if (m_core)
		App::physics.deactivatePhysicsComponentCore(m_core);
	m_core = move.m_core;	move.m_core = nullptr;

	if (m_core)
	{
		m_core->pOwner = m_pOwner;
	}

	return *this;
}

void PhysicsComponent::setOwner(SimulationObject* pNewOwner)
{
	m_pOwner = pNewOwner;
	if (m_core)
		m_core->pOwner = m_pOwner;
}

void PhysicsComponent::setPosition(glm::vec3 position)
{
	m_core->setPosition(position);
}

void PhysicsComponent::setRotation(glm::vec3 rotation)
{
	m_core->setRotation(rotation);
}

void PhysicsComponent::setActive(bool active)
{
	m_core->active = active;
}

bool PhysicsComponent::isActive() const
{
	return m_core->active;
}

void PhysicsComponentCore::setPosition(const glm::vec3& position)
{
	for (auto& [_, collider] : collider2Ds)
		collider.setPosition(position);
}

void PhysicsComponentCore::setRotation(const glm::vec3& rotation)
{
	for (auto& [_, collider] : collider2Ds)
		collider.setRotation(rotation);
}
