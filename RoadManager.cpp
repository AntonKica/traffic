#include "RoadManager.h"
#include "GlobalObjects.h"

RoadManager::RoadManager()
{
	roadCreator.initialize(this);
}

void RoadManager::update()
{
	updateSelectedRoads();
	roadCreator.update();

	auto cursor = App::Scene.m_simArea.getMousePosition();
}

void RoadManager::addRoad(Road road)
{
	roads.emplace_back(road);
}

std::optional<Road*> RoadManager::getSelectedRoad()
{
	return selectedRoad;
}

void RoadManager::updateSelectedRoads()
{
	auto cursor = App::Scene.m_simArea.getMousePosition();
	selectedRoad.reset();

	if (cursor)
	{

		for (auto& road : roads)
		{
			if (road.isPointOnRoad(cursor.value()))
			{
				selectedRoad = &road;
				break;
			}
		}
	}
}
