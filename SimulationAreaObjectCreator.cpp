#include "SimulationAreaObjectCreator.h"
#include "GlobalObjects.h"

#include "SimulationAreaObject.h"
#include "BasicRoad.h"
#include "BasicBuilding.h"
#include "RoadCurve.h"

SimulationAreaObjectCreator::SimulationAreaObjectCreator()
{
	// small hack
	m_simulationAreaPtr = &App::Scene.m_simArea;

	initResources();

	setCreateObject(0);
}

SimulationAreaObjectCreator::~SimulationAreaObjectCreator()
{
	releaseResources();
}

void SimulationAreaObjectCreator::initResources()
{
	int id = 0;
	m_prototypes[id++] = new BasicRoad;
	m_prototypes[id++] = new BasicBuilding;
	m_prototypes[id++] = new RoadCurve;

	m_prototypesCount = id;
}

void SimulationAreaObjectCreator::processKeyInput(int key, int value)
{
	if (key == GLFW_KEY_Q && value == GLFW_PRESS)
		m_currentObject->rotate(Rotation::RotationDirection::LEFT);
	else if (key == GLFW_KEY_R && value == GLFW_PRESS)
		m_currentObject->rotate(Rotation::RotationDirection::RIGHT);
}

void SimulationAreaObjectCreator::setCreateObject(int id)
{
	if (id >= m_prototypesCount)
		throw std::runtime_error("Unknown SimulationAreaObject, and, fix this mess of thousand place creation");

	m_currentObject = m_prototypes[id];
}

const SimulationAreaObject* const SimulationAreaObjectCreator::getCurrentObject() const
{
	return m_currentObject;
}

void SimulationAreaObjectCreator::releaseResources()
{
	for (auto& [id, prototype] : m_prototypes)
		delete prototype;
}

void SimulationAreaObjectCreator::update()
{
	updateCreateObjectPos();
}


void SimulationAreaObjectCreator::updateCreateObjectPos()
{
	auto newPos = m_simulationAreaPtr->getSelectedPointPos();

	if(newPos)
		m_currentObject->setPosition(newPos.value());
}
