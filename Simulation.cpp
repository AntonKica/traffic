#include "Simulation.h"

void Simulation::updateSimulation()
{
	for (auto& object : m_simulationAreaObjects)
	{
		if(object->isActive())
			object->update();
	}
}

void Simulation::registerObject(pSimulationObject toRegister)
{
	m_simulationAreaObjects.emplace_back(toRegister);
}

void Simulation::unregisterObject(pSimulationObject toUnregister)
{
	m_simulationAreaObjects.erase(std::find(std::begin(m_simulationAreaObjects), std::end(m_simulationAreaObjects), toUnregister));
}
