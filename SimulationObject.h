#pragma once
#include <string>
#include <glm/glm.hpp>
#include "GraphicsComponent.h"
#include "PhysicsComponent.h"

class SimulationObject
{
	friend class SimulationArea;
public:
	SimulationObject();
	virtual ~SimulationObject();
	SimulationObject(const SimulationObject& copy);
	SimulationObject(SimulationObject&& move);
	SimulationObject& operator=(const SimulationObject & copy) = default;
	SimulationObject& operator=(SimulationObject && move) = default;

	virtual void update();

	GraphicsComponent& getGraphicsComponent();
	PhysicsComponent& getPhysicsComponent();
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);

	void setupModel(const Info::ModelInfo& modelInfo, bool activateOnCreation);

private:
	GraphicsComponent m_graphicsComponent;
	PhysicsComponent m_physicsComponent;

	glm::vec3 m_position;
	glm::vec3 m_rotation;
};
using pSimulationObject = SimulationObject*;

