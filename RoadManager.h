#pragma once
//#include "SimulationArea.h"
#include "Road.h"
class RoadManager
{
public:
	void addRoad(Road road);
private:
	std::vector<Road> roads;
};

