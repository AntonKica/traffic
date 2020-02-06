#pragma once
#include "SimulationObject.h"
#include "RoadPathFinder.h"
#include "Utilities.h"


class SimpleCar
	: public SimulationObject
{
public:
	void update() override;

	void drive(PathFinding::TravellSegments travellSegments);

private:
	void setupCollision();
	void handleNearbyCars();
	
	glm::vec3 m_currentDirection;
	glm::vec2 m_size;
	float m_collisionSizeMultiplier = 1.0;

	float speed = 1.0f;
	Points pathToTake;
	int currentLine = 0;
	float currentlyTravelled;
	PathFinding::TravellSegments m_travellSegments;
};

