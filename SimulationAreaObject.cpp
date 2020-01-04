#include "SimulationAreaObject.h"

#include "GlobalObjects.h"

#include "PipelinesManager.h"
#include "GraphicsComponent.h"

SimulationAreaObject::SimulationAreaObject()
{
	m_rotation = m_position = glm::vec3(0);
}

SimulationAreaObject::~SimulationAreaObject()
{
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
	graphicsComponent.setPosition(newPosition);
	collider2D.setPosition(newPosition);
}

void SimulationAreaObject::setRotation(const glm::vec3& newRotation)
{
	m_rotation = newRotation;
	graphicsComponent.setRotation(newRotation);
	collider2D.setRotation(newRotation);
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

	graphicsComponent = App::Scene.vulkanBase.createGrahicsComponent(createInfo);
	graphicsComponent.setActive(activateOnCreation);
	updateGraphics();
}

void SimulationAreaObject::updateGraphics()
{
	graphicsComponent.setPosition(m_position);
	graphicsComponent.setRotation(m_rotation);
}
