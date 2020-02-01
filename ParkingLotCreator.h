#pragma once
#include "SimulationObject.h"
#include "ParkingLot.h"

#include <optional>
#include <array>

class BuildingCreator;
class SimulationArea;
class ParkingLotCreator
	: public SimulationObject
{
public:
	ParkingLotCreator(BuildingCreator* pBuildingCreator, SimulationArea* pSimulationArea);

	virtual void update() override;

	virtual void setActiveAction() override;
private:
	void updateCurrentEdges();
	void updatePrototype();
	void updateHelpLines();
	void tryCreatingParkingLot();

	BuildingCreator* m_pBuildingCreator;
	SimulationArea* m_pSimulationArea;

	std::optional<ParkingLot::DirectedEdge> m_currentEdge;
	std::vector<ParkingLot::DirectedEdge> m_settedEdges;
	ParkingLot m_prototype;
};

