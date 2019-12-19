#pragma once
//#include "SimulationArea.h"
#include "Road.h"
#include "RoadCreator.h"
#include <list>

class RoadManger;
class PathVisualizer
{
	Points generateArrows();
public:
	void updateVisuals();

	GraphicsComponent graphics;
};

class RoadManager
{
};

