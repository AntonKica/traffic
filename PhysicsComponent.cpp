#include "PhysicsComponent.h"
#include "GlobalObjects.h"


void PhysicsComponent::setSelfCollisionTags(const std::vector<std::string>& newSelfTags)
{
	Info::PhysicsComponentUpdateTags tags;
	tags.newTags = newSelfTags;

	setCollisionTags(tags);
}

void PhysicsComponent::setOtherCollisionTags(const std::vector<std::string>& newOtherTags)
{
	Info::PhysicsComponentUpdateTags tags;
	tags.newOtherTags = newOtherTags;

	setCollisionTags(tags);
}

void PhysicsComponent::setCollisionTags(const Info::PhysicsComponentUpdateTags& updateInfo)
{
	App::physics.setPhysicsComponentCollisionTags(m_core, updateInfo);
}

bool PhysicsComponent::inCollision() const
{
	for (const auto& [tag, objects] : m_core->inCollisionWith)
	{
		if (!objects.empty())
			return true;
	}

	return false;
}
bool PhysicsComponent::inCollisionWith(std::string tag) const
{
	auto flag = App::physics.getTagFlag(tag);
	for (const auto& [objectsTag, objects] : m_core->inCollisionWith)
	{
		if (App::physics.compatibleTags(flag, objectsTag) && !objects.empty())
			return true;
	}

	return false;
}

std::vector<SimulationObject*> PhysicsComponent::getAllCollisionWith(std::string tag) const
{
	std::set<SimulationObject*> allCollidedObjects;

	auto flag = App::physics.getTagFlag(tag);
	for (const auto& [objectsTag, objects] : m_core->inCollisionWith)
	{
		if (App::physics.compatibleTags(flag, objectsTag) && !objects.empty())
		{
			allCollidedObjects.insert(objects.begin(), objects.end());
		}
	}

	return std::vector(allCollidedObjects.begin(), allCollidedObjects.end());
}

Collider2D& PhysicsComponent::collider()
{
	return m_core->collider2D;
}

const Collider2D& PhysicsComponent::collider() const
{
	return m_core->collider2D;
}

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

	m_core->pOwner = m_pOwner;
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

	m_core->pOwner = m_pOwner;

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

void PhysicsComponent::setOwner(SimulationObject* pNewOwner)
{
	m_pOwner = pNewOwner;
	m_core->pOwner = m_pOwner;
}

void PhysicsComponent::setActive(bool active)
{
	m_core->active = active;
}

bool PhysicsComponent::isActive() const
{
	return m_core->active;
}