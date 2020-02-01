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
		PARKING_LOT,
		MAX_BUILDING_TYPE
	};

	BasicBuilding();

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

