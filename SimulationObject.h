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
	SimulationObject& operator=(const SimulationObject & copy);
	SimulationObject& operator=(SimulationObject && move);

	virtual void update();

	GraphicsComponent& getGraphicsComponent();
	PhysicsComponent& getPhysicsComponent();

	void disableComponents();
	void disableGraphics();
	void disablePhysics();
	void enableComponents();
	void enableGraphics();
	void enablePhysics();


	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	void setPosition(glm::vec3 newPosition);
	void setRotation(glm::vec3 newRotation);
	bool isActive() const;
	void setActive(bool active);

	void setupModel(const Info::ModelInfo& modelInfo, bool activateOnCreation);
	void setupModelWithLines(const Info::ModelInfo& modelInfo, bool activateOnCreation);

protected:
	virtual void setActiveAction();

private:
	struct
	{
		GraphicsComponent graphics;
		PhysicsComponent physics;
	} m_components;

	glm::vec3 m_position;
	glm::vec3 m_rotation;

	bool m_active = false;
	bool m_disableGraphics = false;
	bool m_disablePhysics = false;
};
using pSimulationObject = SimulationObject*;

