#include "ObjectManager.h"
#include "SimulationArea.h"


ObjectManagerUI::ObjectManagerUI(ObjectManager* objectManager)
{
	m_pObjectManager = objectManager;
	setActive(true);
}

void ObjectManagerUI::draw()
{
	auto io = ImGui::GetIO();
	ImGui::Begin("kk");
	ImGui::Text("Road");
	ImGui::End();
}


ObjectManager::ObjectManager(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea), m_roadCreator(this), m_ui(this)
{

}

void ObjectManager::update()
{
	updateSelectedRoad();

	m_roadCreator.update();
}

void ObjectManager::updateSelectedRoad()
{
	auto cursor = m_pSimulationArea->getMousePosition();
	m_selectedRoad.reset();

	const glm::vec4 green = glm::vec4(0.47, 0.98, 0.0, 1.0);
	if (cursor)
	{
		for (auto& road : m_roads.data)
		{
			if (road.sitsOnRoad(cursor.value()))
			{
				m_selectedRoad = &road;
				road.m_graphicsComponent.setTint(green);
			}
			else
			{
				road.m_graphicsComponent.setTint(glm::vec4());
			}
		}
	}
}

std::optional<Road*> ObjectManager::getSelectedRoad() const
{
	return m_selectedRoad;
}