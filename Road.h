#pragma once
#include "SimulationAreaObject.h"

using Point = glm::vec3;
using Points = std::vector<Point>;

bool polygonPointCollision(const Points& polygon, const Point& point);
bool polygonPointCollision(const Points& vertices, float px, float py);
bool polygonPolygonCollision(const Points& polygonOne, const Points& polygonTwo);

class Road :
	public SimulationAreaObject
{
public:
	static Points createLocalPoints(const Points& points, const glm::vec3& position);

	void construct(const Points& points, uint32_t laneCount, float width, std::string texture);
	bool isPointOnRoad(const Point& point) const;
	Point getPointOnRoad(const Point& pointPosition) const;
protected:
	struct RoadParameters
	{
		Points axis;
		Points model;
		uint32_t laneCount;
		float width;
		std::string texture;
	};

	RoadParameters parameters;
public:
	RoadParameters getRoadParameters() const;
};

