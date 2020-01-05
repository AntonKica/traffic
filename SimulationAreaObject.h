#pragma once
#include <string>
#include <glm/glm.hpp>
#include "GraphicsComponent.h"
#include "PhysicsComponent.h"

class SimulationAreaObject
{
	friend class SimulationArea;
public:
	SimulationAreaObject();
	SimulationAreaObject(const SimulationAreaObject& copy);
	SimulationAreaObject(SimulationAreaObject&& move);
	SimulationAreaObject& operator=(const SimulationAreaObject& copy);
	SimulationAreaObject& operator=(SimulationAreaObject&& move);
	virtual ~SimulationAreaObject();

	GraphicsComponent& getGraphicsComponent();
	PhysicsComponent& getPhysicsComponent();
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);

	void setupModel(const Info::ModelInfo& modelInfo, bool activateOnCreation);

	virtual void update();
private:
	GraphicsComponent* m_graphicsComponent = nullptr;
	PhysicsComponent* m_physicsComponent = nullptr;

	glm::vec3 m_position;
	glm::vec3 m_rotation;
};

