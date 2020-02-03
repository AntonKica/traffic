#pragma once
#include "SimulationObject.h"
#include "RoadPathFinder.h"
#include "Utilities.h"


class SimpleCar
	: public SimulationObject
{
private:
	float speed = 1.0f;
	Points pathToTake;
	int currentLine = 0;
	float currentlyTravelled;

public:
	void update() override;

	void drive(PathFinding::TravellSegments travellSegments);
	PathFinding::TravellSegments m_travellSegments;
};

