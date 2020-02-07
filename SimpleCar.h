#pragma once
#include "SimulationObject.h"
#include "RoadPathFinder.h"
#include "BasicGeometry.h"
#include <optional>

class SimpleCar
	: public SimulationObject
{
public:
	void update() override;

	void drive(PathFinding::TravellSegments travellSegments);
	bool finishedDriving() const;
private:
	void advancePath();
	void updateDirectionAndRotation();
	void updatePosition();
	void setupDetection();
	void handleNearbyCars();
	bool inAlertDistance(const SimpleCar& other);
	bool isInFrontOf(const SimpleCar& other);
	bool isOnMyFuturePath(const SimpleCar& other);


	glm::vec3 m_currentDirection;
	glm::vec2 m_size;

	float m_speed = 1.0f;
	Points m_pathToTake;
	PathFinding::TravellSegments m_travellSegments;
};

