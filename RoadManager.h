#pragma once
//#include "SimulationArea.h"
#include "Road.h"
#include "RoadCreator.h"
#include <list>

class RoadManger;
class PathVisualizer
{
	RoadManager* roadManager;

	Points generateArrows();
public:
	void initialize(RoadManager* roadManager);
	void updateVisuals();

	GraphicsComponent graphics;
};

class RoadManager
{
public:
	RoadManager();

	void update(float deltaTime);
	void addRoad(Road road);
	void addRoads(const std::vector<Road>& insertRoads);
	void removeRoad(Road* toRemove);
	void removeRoads(std::vector<Road*> toRemove);
	bool somethingChanged() const;

	RoadCreator roadCreator;
	PathVisualizer pathVisualizer;
	
	//gets first
	std::optional<Road*> getSelectedRoad();
private:

	bool changedRoads = false;
	friend class PathVisualizer;
	void updateSelectedRoads();
	std::list<Road> roads;
	std::optional<Road*> selectedRoad;
};

