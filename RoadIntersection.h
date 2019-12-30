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

	glm::vec3 getDirectionPointFromConnectionPoint(Point point) override;
	ConnectionPossibility getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const override;
private:
	float m_width = 0;
	Point m_centre = {};
	std::vector<Point> m_connectionPoints;
};

