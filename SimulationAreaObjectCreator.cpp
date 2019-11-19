#include "SimulationAreaObjectCreator.h"
#include "GlobalObjects.h"

#include "SimulationAreaObject.h"
#include "BasicRoad.h"
#include "BasicBuilding.h"
#include "RoadCurve.h"

//int static memeber
StaticObjectIdentificators SimulationAreaObjectCreator::s_identificator = StaticObjectIdentificators();

SimulationAreaObjectCreator::SimulationAreaObjectCreator()
{
	s_identificator = {};
	// small hack
	m_simulationAreaPtr = &App::Scene.m_simArea;
	m_prototypesCount = s_identificator.getObjectCount();

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
	StaticObjectIdentificators::SimulationObjectType type = static_cast<StaticObjectIdentificators::SimulationObjectType>(0);
	while(type != StaticObjectIdentificators::SimulationObjectType::MAX_TYPES)
	{
		std::cerr << "Module is being copied" << '\n';
		m_prototypes[type] = s_identificator.getObject(type);
		type = static_cast<StaticObjectIdentificators::SimulationObjectType>(++id);
	}

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

	setCreateObject(static_cast<StaticObjectIdentificators::SimulationObjectType>(id));
}

void SimulationAreaObjectCreator::setCreateObject(StaticObjectIdentificators::SimulationObjectType type)
{
	m_currentObject = m_prototypes[type];
	m_currentType = type;
}

const SimulationAreaObject* const SimulationAreaObjectCreator::getObjectFromType(StaticObjectIdentificators::SimulationObjectType type) const
{
	auto findIT = m_prototypes.find(type);

	if (findIT == std::end(m_prototypes))
		throw std::runtime_error("Unknown identificator id " + std::to_string((int)type));

	return findIT->second;
}

const SimulationAreaObject* const SimulationAreaObjectCreator::getCurrentObject() const
{
	return m_currentObject;
}

StaticObjectIdentificators::SimulationObjectType SimulationAreaObjectCreator::getCurrentType() const
{
	return m_currentType;
}

SimulationAreaObject* SimulationAreaObjectCreator::getRawPointerFromType(StaticObjectIdentificators::SimulationObjectType type) const
{
	switch (type)
	{
	case StaticObjectIdentificators::SimulationObjectType::BUILDING:
		return new BasicBuilding;
	case StaticObjectIdentificators::SimulationObjectType::ROAD:
		return new BasicRoad;
	case StaticObjectIdentificators::SimulationObjectType::CURVED_ROAD:
		return new RoadCurve;
	default:
		throw std::runtime_error("Unknown type in get raw pointer fun()");
	}
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

StaticObjectIdentificators::StaticObjectIdentificators()
{
	setup();
}

StaticObjectIdentificators::~StaticObjectIdentificators()
{
	cleanup();
}

const std::vector<StaticObjectIdentificators::ObjectIdentification>& StaticObjectIdentificators::getIdentificators() const
{
	return identifications;
}

SimulationAreaObject* StaticObjectIdentificators::getObject(SimulationObjectType type)
{
	for (auto& identifications : identifications)
	{
		if (identifications.type == type)
			return identifications.object;
	}

	throw std::runtime_error("Unknown identificator id " + std::to_string((int)type));
}

StaticObjectIdentificators::SimulationObjectType StaticObjectIdentificators::getType(const std::string& name) const
{
	auto findIT = std::find_if(std::begin(identifications), std::end(identifications), 
		[&name](const StaticObjectIdentificators::ObjectIdentification& identificator)
		{
			return identificator.name == name;
		});
	if (findIT == std::end(identifications))
		throw std::runtime_error("Unknown identificator name " + name);

	return findIT->type;
}

std::string StaticObjectIdentificators::getName(SimulationObjectType type) const
{
	auto findIT = std::find_if(std::begin(identifications), std::end(identifications),
		[&type](const StaticObjectIdentificators::ObjectIdentification& identificator)
		{
			return identificator.type == type;
		});
	if (findIT == std::end(identifications))
		throw std::runtime_error("Unknown identificator id " + std::to_string((int)type));

	return findIT->name;
}

size_t StaticObjectIdentificators::getObjectCount() const
{
	return identifications.size();
}

void StaticObjectIdentificators::setup()
{
	identifications =
	{
		{SimulationObjectType::BUILDING,	"Building",		new BasicBuilding},
		{SimulationObjectType::ROAD,		"Road",			new BasicRoad},
		{SimulationObjectType::CURVED_ROAD, "Curved road",	new RoadCurve},
	};
}

void StaticObjectIdentificators::cleanup()
{
	for (auto& identifications : identifications)
		delete identifications.object;
}
