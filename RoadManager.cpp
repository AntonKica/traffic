#include "RoadManager.h"
#include "GlobalObjects.h"

RoadManager::RoadManager()
{
	roadCreator.initialize(this);
}

void RoadManager::update()
{
	roadCreator.update();

	auto cursor = App::Scene.m_simArea.getMousePosition();
	glm::vec4 tintColor(0.1, 0.8, 0.2, 1.0);
	if (cursor)
	{
		for (auto& road : roads)
		{
			if (polygonPointCollision(road.getRoadParameters().model, cursor.value().x, cursor.value().z))
				road.m_graphicsComponent.setTint(tintColor);
			else
				road.m_graphicsComponent.setTint(glm::vec4());
		}
	}
}

void RoadManager::addRoad(Road road)
{
	roads.emplace_back(road);
}
