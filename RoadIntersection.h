#pragma once
#pragma once
#include "SimulationAreaObject.h"
#include "Road.h"
#include "Utilities.h"
#include <vector>
#include <array>

class Road;
class RoadIntersection
	: public SimulationAreaObject
{
private:
	float width = 0;
	Point centre = {};
	std::vector<Point> connectionPoints;
public:
	void construct(std::array<Road*, 3> roads, Point intersectionPoint);
};

