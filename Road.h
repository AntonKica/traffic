#pragma once
#include "SimulationAreaObject.h"
#include "Utilities.h"
#include <vector>

bool polygonPointCollision(const Points& polygon, const Point& point);
bool polygonPointCollision(const Points& vertices, float px, float py);
bool polygonPolygonCollision(const Points& polygonOne, const Points& polygonTwo);

class Road :
	public SimulationAreaObject
{
private:
protected:
	struct RoadParameters
	{
		Points localAxis;
		Points worldAxis;
		Points model;
		uint32_t laneCount;
		std::vector<Points> lanes;
		float width;
		std::string texture;
	};

	RoadParameters parameters;

	void createPaths();
public:
	static Points createLocalPoints(const Points& points, const glm::vec3& position);
	static Road mergeRoads(std::pair<Road, Road> roads);

	void construct(const Points& points, uint32_t laneCount, float width, std::string texture);
	void construct(const Points& localAxis, glm::vec3 position, uint32_t laneCount, float width, std::string texture);
	std::optional<Road> splitRoad(const Point& splitPoint);
	Point shorten(Point endPoint, float cutSize);

	bool isPointOnRoad(const Point& point) const;
	Point getPointOnRoad(const Point& pointPosition) const;
	RoadParameters getRoadParameters() const;

	Points getPath(int path) const;
	uint32_t getLaneCount() const;
};

