#pragma once
#include "BasicCreator.h"
#include "BasicBuilding.h"
#include <map>
#include <memory>

class BuildingCreatorUI
	: public BasicUI
{
public:
	virtual void draw() override;
};


/*class Visualizer
{

};*/

namespace BC
{
	struct Resource
	{
		Resource(std::string modelPath, std::string name, BasicBuilding::BuildingType type);

		std::string name;
		//std::string modelPath;

		BasicBuilding::BuildingType type;
		std::shared_ptr<BasicBuilding> prototype;

	};
}

class BuildingCreator
	: public BasicCreator<BuildingCreatorUI>
{
public:
	BuildingCreator(ObjectManager* objManager);
	void prepareResources();

	void update() override;
protected:
	virtual void setCreatorModeAction() override;
	virtual void setActiveAction() override;

private:
	std::map<BasicBuilding::BuildingType, std::vector<BC::Resource>> m_resources;

	BC::Resource* m_currentResource;
};

