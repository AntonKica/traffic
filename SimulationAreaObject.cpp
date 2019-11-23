#include "SimulationAreaObject.h"

#include "GlobalObjects.h"

#include "DataManager.h"
#include "PipelinesManager.h"
#include "GraphicsComponent.h"

SimulationAreaObject::SimulationAreaObject()
{
	m_rotation = m_position = glm::vec3(0);
}

SimulationAreaObject::~SimulationAreaObject()
{
}

void SimulationAreaObject::place(const glm::vec3& placementPosition, const glm::vec3& rotation)
{
	// use setX rather
	m_position = placementPosition;
	m_rotation = placementPosition;

	updateGraphics();
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
		/*case Rotation::RotationDirection::FORWARD:
			m_rotation.x -= angleQuantum;
			break;
		case Rotation::RotationDirection::BACKWARD:
			m_rotation.x += angleQuantum;
			break;*/
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
}

void SimulationAreaObject::updateGraphics()
{
	if (!m_graphicsComponent.initialized())
		setupModel();

	m_graphicsComponent.setPosition(m_position + getModelPositionOffset());
	m_graphicsComponent.setRotation(m_rotation);
}

void SimulationAreaObject::setupModel()
{
	Info::DrawInfo dInfo{};
	dInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	Info::ModelInfo mInfo{};
	GO::TypedVertices vvs;
	vvs.first = GO::VertexType::DEFAULT;
	for (float i = 0; i <= 6.28; i+= 6.28/4)
	{
		GO::VariantVertex vv;
		vv.vertex.position = { sin(i),0,cos(i)};
		vvs.second.push_back(vv);
	}
	mInfo.vertices = &vvs;

	mInfo.modelPath = getModelPath();

	Info::GraphicsComponentCreateInfo createInfo;
	createInfo.drawInfo = &dInfo;
	createInfo.modelInfo = &mInfo;

	m_graphicsComponent = App::Scene.vulkanBase.createGrahicsComponent(createInfo);
}

glm::vec3 SimulationAreaObject::getModelPositionOffset() const
{
	return glm::vec3();
}
