#include "SimulationObject.h"

#include "GlobalObjects.h"

#include "PipelinesManager.h"
#include "GraphicsComponent.h"

SimulationObject::SimulationObject()
{
	App::simulation.registerObject(this);
	m_components.physics.setOwner(this);
}

SimulationObject::~SimulationObject()
{
	App::simulation.unregisterObject(this);
}

SimulationObject::SimulationObject(const SimulationObject& copy)
{
	App::simulation.registerObject(this);

	m_components.graphics = copy.m_components.graphics;
	m_components.physics = copy.m_components.physics;
	m_components.physics.setOwner(this);

	m_position = copy.m_position;
	m_rotation = copy.m_rotation;

	m_active			= copy.m_active;
	m_disableGraphics	= copy.m_disableGraphics;
	m_disablePhysics	= copy.m_disablePhysics;
}

SimulationObject::SimulationObject(SimulationObject&& move)
{
	App::simulation.registerObject(this);

	m_components.graphics.operator=(std::move(move.m_components.graphics));
	m_components.physics.operator=(std::move(move.m_components.physics));
	m_components.physics.setOwner(this);

	m_position = std::move(move.m_position);
	m_rotation = std::move(move.m_rotation);

	m_active			= std::move(move.m_active);
	m_disableGraphics	= std::move(move.m_disableGraphics);
	m_disablePhysics	= std::move(move.m_disablePhysics);
}

SimulationObject& SimulationObject::operator=(const SimulationObject& copy)
{
	m_components.graphics = copy.m_components.graphics;
	m_components.physics = copy.m_components.physics;
	m_components.physics.setOwner(this);

	m_position = copy.m_position;
	m_rotation = copy.m_rotation;

	m_active = copy.m_active;
	m_disableGraphics = copy.m_disableGraphics;
	m_disablePhysics = copy.m_disablePhysics;

	return *this;
}

SimulationObject& SimulationObject::operator=(SimulationObject&& move)
{
	m_components.graphics.operator=(std::move(move.m_components.graphics));
	m_components.physics.operator=(std::move(move.m_components.physics));
	m_components.physics.setOwner(this);

	m_position = std::move(move.m_position);
	m_rotation = std::move(move.m_rotation);

	m_active = std::move(move.m_active);
	m_disableGraphics = std::move(move.m_disableGraphics);
	m_disablePhysics = std::move(move.m_disablePhysics);


	return *this;
}

void SimulationObject::update()
{
}

GraphicsComponent& SimulationObject::getGraphicsComponent()
{
	return m_components.graphics;
}
PhysicsComponent& SimulationObject::getPhysicsComponent()
{
	return m_components.physics;
}

void SimulationObject::disableComponents()
{
	disableGraphics();
	disablePhysics();
}

void SimulationObject::disableGraphics()
{
	m_disableGraphics = true;
	m_components.graphics.setActive(false);
}

void SimulationObject::disablePhysics()
{
	m_disablePhysics = true;
	m_components.physics.setActive(false);
}

void SimulationObject::enableComponents()
{
	enableGraphics();
	enablePhysics();
}

void SimulationObject::enableGraphics()
{
	m_disableGraphics = false;
	m_components.graphics.setActive(true);
}

void SimulationObject::enablePhysics()
{
	m_disablePhysics = false;
	m_components.physics.setActive(true);
}

glm::vec3 SimulationObject::getPosition() const
{
	return m_position;
}

glm::vec3 SimulationObject::getRotation() const
{
	return m_rotation;
}
void SimulationObject::setPosition(glm::vec3 newPosition)
{
	m_position = newPosition;
	m_components.graphics.setPosition(newPosition);
	m_components.physics.collider().setPosition(newPosition);
}

void SimulationObject::setRotation(glm::vec3 newRotation)
{
	m_rotation = newRotation;
	m_components.graphics.setRotation(newRotation);
	m_components.physics.collider().setRotation(newRotation);
}

bool SimulationObject::isActive() const
{
	return m_active;
}

void SimulationObject::setActive(bool active)
{
	m_active = active;

	m_components.graphics.setActive(m_active && !m_disableGraphics);
	m_components.physics.setActive(m_active && !m_disablePhysics);

	setActiveAction();
}

void SimulationObject::setActiveAction()
{
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

	m_components.graphics.updateGraphicsComponent(createInfo);
	m_components.graphics.setActive(activateOnCreation);
}

void SimulationObject::setupModelWithLines(const Info::ModelInfo& modelInfo, bool activateOnCreation)
{
	Info::DrawInfo dInfo{};
	dInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	dInfo.polygon = VK_POLYGON_MODE_LINE;
	dInfo.lineWidth = 1.0f;

	Info::GraphicsComponentCreateInfo createInfo;
	createInfo.drawInfo = &dInfo;
	createInfo.modelInfo = &modelInfo;

	m_components.graphics.updateGraphicsComponent(createInfo);
	m_components.graphics.setActive(activateOnCreation);
}


