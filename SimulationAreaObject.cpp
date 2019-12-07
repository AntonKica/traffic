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

/*
void SimulationAreaObject::rotate(Rotation::RotationDirection direction)
{
	static const float angleQuantum = 90.0;
	switch (direction)
	{
	case Rotation::RotationDirection::LEFT:
		m_rotation.x -= angleQuantum;
		break;
	case Rotation::RotationDirection::RIGHT:
		m_rotation.x += angleQuantum;
		break;
	case Rotation::RotationDirection::UP:
		m_rotation.y -= angleQuantum;
		break;
	case Rotation::RotationDirection::DOWN:
		m_rotation.y += angleQuantum;
		break;
	default:
		throw std::runtime_error("Unknowns rotation");
	}

	auto fixRotation = [](float angle) -> float
	{
		if (angle < -180.0f)
			return 180.0f;
		else if (angle > 180.0f)
			return -180.0f;
		else
			return angle;
	};
	std::transform((float*)&m_rotation.x, (float*)&m_rotation.x + 3, (float*)&m_rotation.x, fixRotation);
}*/

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

	Info::GraphicsComponentCreateInfo createInfo;
	createInfo.drawInfo = &dInfo;
	createInfo.modelInfo = &modelInfo;

	m_graphicsComponent = App::Scene.vulkanBase.createGrahicsComponent(createInfo);
	m_graphicsComponent.setActive(activateOnCreation);
	updateGraphics();
}
