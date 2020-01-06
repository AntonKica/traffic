#include "BuildingCreator.h"
#include "ObjectManager.h"
#include "SimulationArea.h"

#include "GlobalObjects.h"
#include "House.h"

namespace
{
	struct Rect
	{
		glm::vec3 p1, p2, p3, p4;
	};
	Rect deduceRectFromModel(const Model& model)
	{
		glm::vec3 max;
		glm::vec3 min;
		max.x = std::numeric_limits<float>::min();
		max.y = std::numeric_limits<float>::min();
		max.z = std::numeric_limits<float>::min();
		min.y= std::numeric_limits<float>::max();
		min.x = std::numeric_limits<float>::max();
		min.z = std::numeric_limits<float>::max();

		glm::vec3 average = {};
		uint32_t totalPoints = 0;
		for (const auto& mesh : model.meshes)
		{
			totalPoints += mesh.vertices.positions.size();
			for (const auto& p : mesh.vertices.positions)
			{
				average += p;
				max.x = std::max(max.x, p.x);
				max.y = std::max(max.y, p.y);
				max.z = std::max(max.z, p.z);
				min.x = std::min(min.x, p.x);
				min.y = std::min(min.y, p.y);
				min.z = std::min(min.z, p.z);
			}
		}
		average /= float(totalPoints);
		auto getGreater = [](float a, float b)
		{return a > b ? a : b; };

		float absX = getGreater(std::abs(min.x - average.x), std::abs(max.x- average.x));
		float absZ = getGreater(std::abs(min.z - average.z), std::abs(max.z - average.z));

		Rect rect = {
			{ absX, 0.0f, absZ},
			{ absX, 0.0f, absZ},
			{-absX, 0.0f,-absZ},
			{-absX, 0.0f,-absZ}
		};

		return rect;
	}
}
void BuildingCreatorUI::draw()
{

}

BC::Resource::Resource(std::string modelPath, std::string name, BasicBuilding::BuildingType type)
{
	this->name = name;
	this->type = type;
	switch (type)
	{
	case BasicBuilding::BuildingType::HOUSE:
		House house;
		house.create(glm::vec3(), modelPath);
		house.getGraphicsComponent().setActive(false);

		this->prototype = std::shared_ptr<BasicBuilding>(new House(house));
		break;
	}
	Rect rect = deduceRectFromModel(Model(modelPath));
	this->prototype->getPhysicsComponent().collider().setBoundaries({ rect.p1, rect.p2, rect.p3, rect.p4 });
}

BuildingCreator::BuildingCreator(ObjectManager* objManager)
	:BasicCreator(objManager)
{
}

void BuildingCreator::prepareResources()
{
	using namespace BC;
	// house
	BasicBuilding::BuildingType curType = BasicBuilding::BuildingType::HOUSE;
	std::vector<Resource> resources
	{
		{ Resource("resources/models/house/house.obj","House 1", curType)},
	};
	m_resources[curType] = resources;

	m_currentResource = &m_resources.begin()->second.front();
}

void BuildingCreator::update()
{
	if (!m_active)
		return;
	
	if (m_currentMode == Creator::CreatorMode::CREATE)
	{
		auto optPosition = m_pObjectManager->m_pSimulationArea->getMousePosition();
		if (optPosition)
		{
			m_currentResource->prototype->getGraphicsComponent().setActive(true);
			m_currentResource->prototype->setPosition(optPosition.value());
		}
		else
		{
			m_currentResource->prototype->getGraphicsComponent().setActive(false);
		}
	}

	if (App::input.pressedLMB())
	{
		switch (m_currentResource->type)
		{
		case BasicBuilding::BuildingType::HOUSE:
			House house = *static_cast<House*>(&*m_currentResource->prototype);

			new House(house);
			break;
		}
	}
}

void BuildingCreator::setCreatorModeAction()
{
	m_currentResource->prototype->getGraphicsComponent().setActive(
		m_currentMode == Creator::CreatorMode::CREATE);

}

void BuildingCreator::setActiveAction()
{
	m_currentResource->prototype->getGraphicsComponent().setActive(m_active);
}

