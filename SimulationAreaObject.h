#pragma once
#include <string>
#include <glm/glm.hpp>
#include "GraphicsComponent.h"

class SimulationAreaObject
{
	friend class SimulationArea;
public:
	SimulationAreaObject();
	virtual ~SimulationAreaObject();

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);

	GraphicsComponent m_graphicsComponent;
protected:
	void updateGraphics();
	void setupModel(const Info::ModelInfo& modelInfo, bool activateOnCreation);

	glm::vec3 m_position;
	glm::vec3 m_rotation;
};

