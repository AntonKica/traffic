#pragma once
//#include "SimulationArea.h"
#include "Road.h"
#include "RoadCreator.h"

class RoadManager
{
public:
	RoadManager();

	void update();
	void addRoad(Road road);

	RoadCreator roadCreator;
private:
	std::vector<Road> roads;
};

