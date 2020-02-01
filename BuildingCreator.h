#pragma once
#include "BasicCreator.h"
#include "BasicBuilding.h"
#include "SimulationObject.h"
#include "ParkingLotCreator.h"

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
class SimulationArea;
//class BuildingCreator;
class PlacementRectangle
	: public SimulationObject
{
public:
	PlacementRectangle(SimulationArea* pSimulationArea);
	virtual void update() override;

	void setSize(glm::dvec2 newSize);
	glm::dvec3 getPaddedPosition() const;
	glm::dvec3 getPaddedRotation() const;
	bool paddedPosition() const;
private:
	void updatePosition();
	void updateVertices();
	void updateGraphicsData();
	void updatePhysicsData();
	void updatePaddings();

	bool m_isPositionPadded = false;
	bool m_changedSize = false;
	glm::dvec2 m_rectangleSize;
	std::array<Point, 4> m_vertices;

	glm::dvec3 m_paddedPosition;
	glm::dvec3 m_paddedRotation;

	SimulationArea* m_pSimulationArea;
};

class BuildingCreator
	: public BasicCreator<BuildingCreatorUI>
{
public:
	BuildingCreator(ObjectManager* objManager);
	void prepareResources();
	void setResource(BC::Resource* resource);

	void update();
protected:
	virtual void setCreatorModeAction() override;
	virtual void setActiveAction() override;

private:
	std::map<BasicBuilding::BuildingType, std::vector<BC::Resource>> m_resources;

	PlacementRectangle m_placementRectangle;
	BC::Resource* m_currentResource;
	ParkingLotCreator m_parkingLotCreator;
};

