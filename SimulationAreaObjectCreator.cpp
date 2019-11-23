#include "SimulationAreaObjectCreator.h"
#include "GlobalObjects.h"

#include "SimulationAreaObject.h"
#include "BasicRoad.h"
#include "BasicBuilding.h"
#include "RoadCurve.h"

SimulationAreaObjectCreator::SimulationObjectType& operator++(SimulationAreaObjectCreator::SimulationObjectType& type)
{
	return type = static_cast<SimulationAreaObjectCreator::SimulationObjectType>(static_cast<int>(type) + 1);
}

//int static memeber
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
	m_identifications =
	{
		{SimulationObjectType::BUILDING,	"Building",		std::make_shared<BasicBuilding>()},
		{SimulationObjectType::ROAD,		"Road",			std::make_shared<BasicRoad>()},
		{SimulationObjectType::CURVED_ROAD, "Curved road",	std::make_shared<RoadCurve>()},
	};

	m_prototypesCount = m_identifications.size();;
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

	setCreateObject(static_cast<SimulationObjectType>(id));
}

void SimulationAreaObjectCreator::setCreateObject(SimulationObjectType type)
{
	if(m_currentObject)
		m_currentObject->m_graphicsComponent.setActive(false);

	m_currentObject = getObjectFromType(type);
	m_currentObject->m_graphicsComponent.setActive(true);

	m_currentType = type;

	updateCreateObjectPos();
}

const std::vector<SimulationAreaObjectCreator::ObjectIdentification>& SimulationAreaObjectCreator::getIdentifications() const
{
	return m_identifications;
}

std::shared_ptr<SimulationAreaObject> SimulationAreaObjectCreator::getObjectFromType(SimulationObjectType type) const
{
	return getObjectFromType(type);
}

std::shared_ptr<SimulationAreaObject> SimulationAreaObjectCreator::getCurrentObject() const
{
	return m_currentObject;
}

SimulationAreaObjectCreator::SimulationObjectType SimulationAreaObjectCreator::getCurrentType() const
{
	return m_currentType;
}

SimulationAreaObject* SimulationAreaObjectCreator::getRawPointerFromType(SimulationObjectType type) const
{
	switch (type)
	{
	case SimulationObjectType::BUILDING:
		return new BasicBuilding;
	case SimulationObjectType::ROAD:
		return new BasicRoad;
	case SimulationObjectType::CURVED_ROAD:
		return new RoadCurve;
	default:
		throw std::runtime_error("Unknown type in get raw pointer fun()");
	}
}

void SimulationAreaObjectCreator::releaseResources()
{
	m_identifications.clear();
}

void SimulationAreaObjectCreator::update()
{
	updateCreateObjectPos();
}


void SimulationAreaObjectCreator::updateCreateObjectPos()
{
	auto newPos = m_simulationAreaPtr->getSelectedPointPos();

	if (newPos)
	{
		m_currentObject->m_graphicsComponent.setActive(true);
		m_currentObject->setPosition(newPos.value());
	}
	else
	{
		m_currentObject->m_graphicsComponent.setActive(false);
	}
}

std::shared_ptr<SimulationAreaObject> SimulationAreaObjectCreator::getObjectFromType(SimulationObjectType type)
{
	for (auto& identification : m_identifications)
	{
		if (identification.type == type)
			return std::shared_ptr(identification.object);
	}

	throw std::runtime_error("Unknown identificator id " + std::to_string((int)type));
}
