#include "BuildingCreator.h"
#include "GlobalObjects.h"
#include "House.h"

void BuildingCreatorUI::draw()
{

}

BuildingCreator::BuildingCreator(ObjectManager* objManager)
	:BasicCreator(objManager)
{
}

void BuildingCreator::clickEvent()
{
	House* newHouse = new House;

	newHouse->create();
	newHouse->setPosition(App::Scene.m_simArea.getMousePosition().value());
}
