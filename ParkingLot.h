#pragma once
#include "BasicBuilding.h"

class ParkingLot
	: public BasicBuilding
{
public:
	struct DirectedEdge
	{
		Point position = {};
		glm::vec3 direction = {};
	};

	void construct(std::array<DirectedEdge, 2> creationEdges);
	std::array<Point, 4> generateEdgePoints(const std::array<DirectedEdge, 2>& creationEdges);
};

