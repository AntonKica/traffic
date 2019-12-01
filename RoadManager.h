#pragma once
//#include "SimulationArea.h"
#include "Road.h"
#include "RoadCreator.h"
#include <list>

class RoadManager
{
public:
	RoadManager();

	void update();
	void addRoad(Road road);
	void addRoads(const std::vector<Road>& insertRoads);
	void removeRoad(Road* toRemove);
	void removeRoads(std::vector<Road*> toRemove);

	RoadCreator roadCreator;
	
	//gets first
	std::optional<Road*> getSelectedRoad();
private:
	void updateSelectedRoads();
	std::list<Road> roads;
	std::optional<Road*> selectedRoad;
};

