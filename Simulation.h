#pragma once
#include "SimulationObject.h"
#include <list>
class Simulation
{
public:
	void updateSimulation();

	void registerObject(pSimulationObject toRegister);
	void unregisterObject(pSimulationObject toUnregister);
private:
	std::list<pSimulationObject> m_simulationAreaObjects;
};

