#pragma once
#include <string>
#include <glm/glm.hpp>
#include "GraphicsComponent.h"

namespace Rotation
{
	enum class RotationDirection
	{
		LEFT,
		RIGHT,
		//FORWARD,
		//BACKWARD,
		DOWN,
		UP
	};
}
class SimulationAreaObject
{
	friend class SimulationArea;
public:
	SimulationAreaObject();
	virtual ~SimulationAreaObject();

	void place(const glm::vec3& placementPosition, const glm::vec3& rotation);
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);

	void rotate(Rotation::RotationDirection direction);

	GraphicsComponent m_graphicsComponent;
protected:
	void updateGraphics();
	void setupModel();
	virtual std::string getModelPath() const = 0;
	virtual glm::vec3 getModelPositionOffset() const;

	glm::vec3 m_position;
	glm::vec3 m_rotation;
};

