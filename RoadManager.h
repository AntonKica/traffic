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
	
	//gets first
	std::optional<Road*> getSelectedRoad();
private:
	void updateSelectedRoads();
	std::vector<Road> roads;
	std::optional<Road*> selectedRoad;
};

