#pragma once
#include <map>
#include "SimulationAreaObject.h"

class SimulationArea;
class SimulationAreaObjectCreator
{
public:
	SimulationAreaObjectCreator();
	~SimulationAreaObjectCreator();

	void update();

	void processKeyInput(int key, int value);
	void setCreateObject(int id);
	const SimulationAreaObject* const getCurrentObject() const;
private:
	void initResources();
	void releaseResources();

	void updateCreateObjectPos();

	std::map<int, SimulationAreaObject*> m_prototypes;

	SimulationArea* m_simulationAreaPtr;

	SimulationAreaObject* m_currentObject;
	int m_prototypesCount;
};

