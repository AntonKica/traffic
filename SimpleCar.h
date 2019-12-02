#pragma once
#include "SimulationAreaObject.h"
#include "Utilities.h"

class SimpleCar
	: public SimulationAreaObject
{
private:
	float speed = 1.0f;
	Points pathToTake;
	int currentLine = 0;
	float currentlyTravelled;

public:
	void update(float deltaTime);

	void create(const Points& pathPoints);
};

