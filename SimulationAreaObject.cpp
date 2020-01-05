#include "SimulationAreaObject.h"

#include "GlobalObjects.h"

#include "PipelinesManager.h"
#include "GraphicsComponent.h"

SimulationAreaObject::SimulationAreaObject()
{
	m_graphicsComponent = GraphicsComponent::createGraphicsComponent();
	m_physicsComponent = PhysicsComponent::createPhysicsComponent();

	m_rotation = m_position = glm::vec3(0);
}

SimulationAreaObject::SimulationAreaObject(const SimulationAreaObject& copy)
{
	m_position = copy.m_position;
	m_rotation = copy.m_rotation;

	m_graphicsComponent = GraphicsComponent::copyGraphicsComponent(copy.m_graphicsComponent);
	m_physicsComponent = PhysicsComponent::copyPhysicsComponent(copy.m_physicsComponent);
}

SimulationAreaObject::SimulationAreaObject(SimulationAreaObject&& move)
{
	m_position = std::move(move.m_position);
	m_rotation = move.m_rotation;

	m_graphicsComponent = move.m_graphicsComponent;
	m_physicsComponent = move.m_physicsComponent;

	move.m_graphicsComponent = nullptr;
	move.m_physicsComponent = nullptr;
}

SimulationAreaObject& SimulationAreaObject::operator=(const SimulationAreaObject& copy)
{
	m_position = copy.m_position;
	m_rotation = copy.m_rotation;

	if(m_graphicsComponent)
		*m_graphicsComponent = *copy.m_graphicsComponent;
	else if(copy.m_graphicsComponent)
		m_graphicsComponent = GraphicsComponent::copyGraphicsComponent(copy.m_graphicsComponent);

	if(m_physicsComponent)
		*m_physicsComponent = *copy.m_physicsComponent;
	else if (copy.m_physicsComponent)
		m_physicsComponent = PhysicsComponent::copyPhysicsComponent(copy.m_physicsComponent);

	return *this;
}

SimulationAreaObject& SimulationAreaObject::operator=(SimulationAreaObject&& move)
{
	m_position = std::move(move.m_position);
	m_rotation = move.m_rotation;

	if(m_graphicsComponent)
		GraphicsComponent::destroyGraphicsComponent(m_graphicsComponent);
	m_graphicsComponent = move.m_graphicsComponent;
	move.m_graphicsComponent = nullptr;

	if (m_physicsComponent)
		PhysicsComponent::destroyPhysicsComponent(m_physicsComponent);
	m_physicsComponent = move.m_physicsComponent;
	move.m_physicsComponent = nullptr;

	return *this;
}

SimulationAreaObject::~SimulationAreaObject()
{
	GraphicsComponent::destroyGraphicsComponent(m_graphicsComponent);
	PhysicsComponent::destroyPhysicsComponent(m_physicsComponent);
}
GraphicsComponent& SimulationAreaObject::getGraphicsComponent()
{
	return *m_graphicsComponent;
}
PhysicsComponent& SimulationAreaObject::getPhysicsComponent()
{
	return *m_physicsComponent;
}
glm::vec3 SimulationAreaObject::getPosition() const
{
	return m_position;
}

glm::vec3 SimulationAreaObject::getRotation() const
{
	return m_rotation;
}
void SimulationAreaObject::setPosition(const glm::vec3& newPosition)
{
	m_position = newPosition;
	m_graphicsComponent->setPosition(newPosition);
	m_physicsComponent->collider().setPosition(newPosition);
}

void SimulationAreaObject::setRotation(const glm::vec3& newRotation)
{
	m_rotation = newRotation;
	m_graphicsComponent->setRotation(newRotation);
	m_physicsComponent->collider().setRotation(newRotation);
}

void SimulationAreaObject::setupModel(const Info::ModelInfo& modelInfo, bool activateOnCreation)
{
	Info::DrawInfo dInfo{};
	dInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	dInfo.polygon = VK_POLYGON_MODE_FILL;
	dInfo.lineWidth = 1.0f;

	Info::GraphicsComponentCreateInfo createInfo;
	createInfo.drawInfo = &dInfo;
	createInfo.modelInfo = &modelInfo;

	m_graphicsComponent->updateGraphicsComponent(createInfo);
	m_graphicsComponent->setActive(activateOnCreation);
}

void SimulationAreaObject::update()
{
}

