#pragma once
#include <map>
#include <memory>
#include <vector>
#include "SimulationAreaObjectStatic.h"

class SimulationArea;
class SimulationAreaObject;


// shoul be called staticObjectCreator
class SimulationAreaObjectCreator
{
public:
	enum class SimulationObjectType
	{
		BUILDING,
		ROAD,
		CURVED_ROAD,
		MAX_TYPES
	};
	friend SimulationObjectType& operator++(SimulationObjectType& type);

	struct ObjectIdentification
	{
		SimulationObjectType type;
		std::string name;
		std::shared_ptr<SimulationAreaObject> object;
	};

	SimulationAreaObjectCreator();
	~SimulationAreaObjectCreator();

	void update();

	void processKeyInput(int key, int value);
	void setCreateObject(int id);
	void setCreateObject(SimulationObjectType type);

	const std::vector<ObjectIdentification>& getIdentifications() const;
	std::shared_ptr<SimulationAreaObject> getObjectFromType(SimulationObjectType type) const;

	std::shared_ptr<SimulationAreaObject> getCurrentObject() const;
	SimulationObjectType getCurrentType() const;
	SimulationAreaObject* getRawPointerFromType(SimulationObjectType type) const;

private:
	void initResources();
	void releaseResources();

	void updateCreateObjectPos();
	std::shared_ptr<SimulationAreaObject> getObjectFromType(SimulationObjectType type);

	std::vector<ObjectIdentification> m_identifications;

	SimulationArea* m_simulationAreaPtr;
	std::shared_ptr<SimulationAreaObject> m_currentObject;

	SimulationObjectType m_currentType;
	int m_prototypesCount;
};