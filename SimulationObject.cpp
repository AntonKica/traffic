#include "SimulationObject.h"

#include "GlobalObjects.h"

#include "PipelinesManager.h"
#include "GraphicsComponent.h"

SimulationObject::SimulationObject()
{
	App::simulation.registerObject(this);
}

SimulationObject::~SimulationObject()
{
	App::simulation.unregisterObject(this);
}

SimulationObject::SimulationObject(const SimulationObject& copy)
{
	App::simulation.registerObject(this);

	m_graphicsComponent = copy.m_graphicsComponent;
	m_physicsComponent = copy.m_physicsComponent;

	m_position = copy.m_position;
	m_rotation = copy.m_rotation;
}

SimulationObject::SimulationObject(SimulationObject&& move)
{
	App::simulation.registerObject(this);

	m_graphicsComponent.operator=(std::move(m_graphicsComponent));
	m_physicsComponent.operator=(std::move(m_physicsComponent));

	m_position = std::move(move.m_position);
	m_rotation = std::move(move.m_rotation);
}

void SimulationObject::update()
{
}

GraphicsComponent& SimulationObject::getGraphicsComponent()
{
	return m_graphicsComponent;
}
PhysicsComponent& SimulationObject::getPhysicsComponent()
{
	return m_physicsComponent;
}
glm::vec3 SimulationObject::getPosition() const
{
	return m_position;
}

glm::vec3 SimulationObject::getRotation() const
{
	return m_rotation;
}
void SimulationObject::setPosition(const glm::vec3& newPosition)
{
	m_position = newPosition;
	m_graphicsComponent.setPosition(newPosition);
	m_physicsComponent.collider().setPosition(newPosition);
}

void SimulationObject::setRotation(const glm::vec3& newRotation)
{
	m_rotation = newRotation;
	m_graphicsComponent.setRotation(newRotation);
	m_physicsComponent.collider().setRotation(newRotation);
}

void SimulationObject::setupModel(const Info::ModelInfo& modelInfo, bool activateOnCreation)
{
	Info::DrawInfo dInfo{};
	dInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	dInfo.polygon = VK_POLYGON_MODE_FILL;
	dInfo.lineWidth = 1.0f;

	Info::GraphicsComponentCreateInfo createInfo;
	createInfo.drawInfo = &dInfo;
	createInfo.modelInfo = &modelInfo;

	m_graphicsComponent.updateGraphicsComponent(createInfo);
	m_graphicsComponent.setActive(activateOnCreation);
}


