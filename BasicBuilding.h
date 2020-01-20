#pragma once
#include "SimulationObject.h"
#include <string>

class Road;
class BasicBuilding :
	public SimulationObject
{
public:
	enum class BuildingType
	{
		HOUSE,
		MAX_BUILDING_TYPE
	};

	BasicBuilding();

	virtual void create(glm::vec3 position, std::string modelPath) = 0;

	Road* getNearbyRoad() const;
	void resetNearbyRoad();
	void setNearbyRoad(Road* nearbyRoad, Point entryPoint);
protected:
	struct NearbyRoad
	{
		Road* road = nullptr;
		Point entryPoint = {};
	};

	NearbyRoad m_nearbyRoad;
};

