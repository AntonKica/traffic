#pragma once
#include "SimulationObject.h"
#include "UI.h"

// predefinitions
class SimulationArea;
class BasicRoad;
class Road;
class RoadIntersection;
class CarSpawner;

namespace Inspector
{
	enum class InspectorMode
	{
		INSPECT,
		EDIT
	};
}
class RoadInspectorUI
	: public UIElement
{
public:
	virtual void draw() override;

	virtual void setActiveAction() override;
	void setInspectElement(BasicRoad* inspected);
	void resetInspectElement();
	void setMode(Inspector::InspectorMode mode);
private:
	void inspectCurrent();
	void inspectRoad(Road* road);
	void inspectIntersection(RoadIntersection* roadIntersection);
	void inspectSpawner(CarSpawner* carSpawner);

	void editCurrent();
	void editRoad(Road* road);
	void editIntersection(RoadIntersection* roadIntersection);
	void editSpawner(CarSpawner* carSpawner);

	std::optional<BasicRoad*> m_currentlyInspected;
	Inspector::InspectorMode m_mode;

	bool m_resetPositionOnNextDraw = false;
};

class RoadInspector
	: public SimulationObject
{
public:
	RoadInspector(SimulationArea* pSimulationArea);
	virtual void update();
	virtual void setActiveAction();

	void setInspectorMode(Inspector::InspectorMode inspectorMode);
private:
	void processInput();
	void updateCurrentlyInspected(); 

	SimulationArea* m_pSimulationArea;
	RoadInspectorUI m_inspectorUI;

	BasicRoad* m_currentlyInspected = nullptr;
	Inspector::InspectorMode m_currentInspectorMode;
};

