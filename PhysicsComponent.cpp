#include "PhysicsComponent.h"
#include "GlobalObjects.h"


PhysicsComponent::PhysicsComponent()
{
	m_core = App::physics.createPhysicsComponentCore();
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
		m_core = App::physics.copyCreatePhysicsComponentCore(copy.m_core);
}

PhysicsComponent::PhysicsComponent(PhysicsComponent&& move) noexcept
{
	if (this == &move)
		return;

	m_core = move.m_core;	move.m_core = nullptr;
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

	return *this;
}

PhysicsComponent& PhysicsComponent::operator=(PhysicsComponent&& move) noexcept
{
	if (this == &move)
		return *this;

	if (m_core)
		App::physics.deactivatePhysicsComponentCore(m_core);
	m_core = move.m_core;	move.m_core = nullptr;

	return *this;
}

void PhysicsComponent::updateSelfCollisionTags(const std::vector<std::string>& newSelfTags)
{
	Info::PhysicsComponentUpdateTags tags;
	tags.newTags = newSelfTags;

	updateCollisionTags(tags);
}

void PhysicsComponent::updateOtherCollisionTags(const std::vector<std::string>& newOtherTags)
{
	Info::PhysicsComponentUpdateTags tags;
	tags.newOtherTags = newOtherTags;

	updateCollisionTags(tags);
}

void PhysicsComponent::updateCollisionTags(const Info::PhysicsComponentUpdateTags& updateInfo)
{
	App::physics.updatePhysicsComponentCollisionTags(m_core, updateInfo);
}

void PhysicsComponent::setActive(bool active)
{
	m_core->active = active;
}

bool PhysicsComponent::active() const
{
	return m_core->active;
}

bool PhysicsComponent::inCollision() const
{
	return m_core->inCollisionWith != 0;
}

bool PhysicsComponent::inCollision(std::string tag) const
{
	return m_core->inCollisionWith & App::physics.getTagFlag(tag);
}

Collider2D& PhysicsComponent::collider()
{
	return m_core->collider2D;
}

const Collider2D& PhysicsComponent::collider() const
{
	return m_core->collider2D;
}
