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
public:
	struct RoadParameters
	{
		struct 
		{
			Points local;
			Points world;
		} axis;
		uint32_t laneCount;
		float width;
		std::string texture;
	};
public:
	// Getters
	RoadParameters getParameters() const;
	Point getHead() const;
	Point getTail() const;
	bool sitsOnHead(const Point& point);
	bool sitsOnTail(const Point& point);
	bool sitsOnRoad(const Point& point);

	void reverseBody();

	Point getPointOnRoad(const Point& point);

	//
	void construct(Points axisPoints, uint32_t laneCount, float width, std::string texture);
	void mergeWithRoad(Road& road);
	Point shorten(const Point& roadEnd, float size);
private:

	using Path = Points;
	void createPaths();

	RoadParameters m_parameters;
	Points m_modelShape;
	// just for cause
public:
	std::vector<Path> m_paths;
};


