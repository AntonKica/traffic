#pragma once
#include <map>
#include <memory>
#include <vector>
#include "SimulationAreaObjectStatic.h"

class SimulationArea;
class SimulationAreaObject;

class StaticObjectIdentificators
{
	// idk, not liking that
public:
	enum class SimulationObjectType
	{
		BUILDING,
		ROAD,
		CURVED_ROAD,
		MAX_TYPES
	};
	struct ObjectIdentification
	{
		SimulationObjectType type;
		std::string name;
		SimulationAreaObject* object;
	};

	StaticObjectIdentificators();
	~StaticObjectIdentificators();

	const std::vector<ObjectIdentification>& getIdentificators() const;
	//std::vector<uint32_t> getIDs() const;
	SimulationAreaObject* getObject(SimulationObjectType type);
	SimulationObjectType getType(const std::string& name) const;
	std::string getName(SimulationObjectType ID) const;
	size_t getObjectCount() const;

private:
	void setup();
	void cleanup();

	//static const constexpr size_t ObjectCount = 8;
	std::vector<ObjectIdentification> identifications;
};

// shoul be called staticObjectCreator
class SimulationAreaObjectCreator
{
public:
	SimulationAreaObjectCreator();
	~SimulationAreaObjectCreator();

	void update();

	void processKeyInput(int key, int value);
	void setCreateObject(int id);
	void setCreateObject(StaticObjectIdentificators::SimulationObjectType type);
	const SimulationAreaObject* const getObjectFromType(StaticObjectIdentificators::SimulationObjectType type) const;
	const SimulationAreaObject* const getCurrentObject() const;
	StaticObjectIdentificators::SimulationObjectType getCurrentType() const;
	SimulationAreaObject* getRawPointerFromType(StaticObjectIdentificators::SimulationObjectType type) const;

	static StaticObjectIdentificators s_identificator;
private:
	void initResources();
	void releaseResources();

	void updateCreateObjectPos();

	std::map<StaticObjectIdentificators::SimulationObjectType, SimulationAreaObject*> m_prototypes;

	SimulationArea* m_simulationAreaPtr;
	SimulationAreaObject* m_currentObject;

	StaticObjectIdentificators::SimulationObjectType m_currentType;
	int m_prototypesCount;
};