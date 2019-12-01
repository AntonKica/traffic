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

void RoadManager::addRoads(const std::vector<Road>& insertRoads)
{
	roads.insert(std::end(roads), std::begin(insertRoads), std::end(insertRoads));
}

void RoadManager::removeRoad(Road* toRemove)
{
	std::list<Road>::iterator removeIter = roads.begin();
	for (removeIter = roads.begin(); removeIter != roads.end(); ++removeIter)
	{
		if (&(*removeIter) == toRemove)
			break;
	}

	roads.erase(removeIter);
}

void RoadManager::removeRoads(std::vector<Road*> toRemove)
{
	for (auto& road : toRemove)
		removeRoad(road);
}

std::optional<Road*> RoadManager::getSelectedRoad()
{
	return selectedRoad;
}



void RoadManager::updateSelectedRoads()
{
	auto cursor = App::Scene.m_simArea.getMousePosition();
	selectedRoad.reset();

	const glm::vec4 green = glm::vec4(0.47, 0.98, 0.0, 1.0);
	if (cursor)
	{
		for (auto& road : roads)
		{
			if (road.isPointOnRoad(cursor.value()))
			{
				selectedRoad = &road;
				road.m_graphicsComponent.setTint(green);
				//break;
			}
			else
			{
				road.m_graphicsComponent.setTint(glm::vec4());
			}
		}
	}
}
