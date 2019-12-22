#pragma once
#pragma once
#include "BasicRoad.h"
#include "Utilities.h"
#include <vector>
#include <array>

class Road;
class RoadIntersection
	: public BasicRoad
{

public:
	void construct(std::array<Road*, 3> roads, Point intersectionPoint);

	virtual glm::vec3 getConnectionDirectionPoint(BasicRoad* road);

private:
	float m_width = 0;
	Point m_centre = {};
	std::vector<Point> m_connectionPoints;
};

