#pragma once
#include "SimulationAreaObject.h"

using Point = glm::vec3;
using Points = std::vector<Point>;


class Road :
	public SimulationAreaObject
{
private:

public:
	static Points createLocalPoints(const Points& points, const glm::vec3& position);

	void construct(const Points& points, uint32_t laneCount, float width, std::string texture);
protected:
	struct RoadParameters
	{
		Points axis;
		uint32_t laneCount;
		float width;
		std::string texture;
	};

	RoadParameters parameters;
};

