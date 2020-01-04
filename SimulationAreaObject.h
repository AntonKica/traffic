#pragma once
#include <string>
#include <glm/glm.hpp>
#include "GraphicsComponent.h"
#include "Collider2D.h"

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

	void setupModel(const Info::ModelInfo& modelInfo, bool activateOnCreation);

	GraphicsComponent graphicsComponent;
	Collider2D collider2D;
private:
	void updateGraphics();

	glm::vec3 m_position;
	glm::vec3 m_rotation;
};

