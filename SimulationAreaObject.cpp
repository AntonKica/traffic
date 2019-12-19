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
	updateGraphics();
}

void SimulationAreaObject::setRotation(const glm::vec3& newRotation)
{
	m_rotation = newRotation;
	updateGraphics();
}

void SimulationAreaObject::updateGraphics()
{
	if (m_graphicsComponent.initialized())
	{
		m_graphicsComponent.setPosition(m_position);
		m_graphicsComponent.setRotation(m_rotation);
	}
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

	m_graphicsComponent = App::Scene.vulkanBase.createGrahicsComponent(createInfo);
	m_graphicsComponent.setActive(activateOnCreation);
	updateGraphics();
}
