#include "BasicBuilding.h"
#include "Road.h"


BasicBuilding::BasicBuilding()
{
}

Road* BasicBuilding::getNearbyRoad() const
{
	return m_nearbyRoad.road;
}

void BasicBuilding::resetNearbyRoad()
{
	m_nearbyRoad = {};
}
void BasicBuilding::setNearbyRoad(Road* nearbyRoad, Point entryPoint)
{
	m_nearbyRoad.road = nearbyRoad;
	m_nearbyRoad.entryPoint = entryPoint;
}
