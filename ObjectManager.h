#pragma once
#include "BuildingCreator.h"
#include "RoadCreator.h"
#include "House.h"
#include "CarSpawner.h"

#include "UI.h"
#include <list>

template <class Type> class Container
{
public:
	using iter = typename std::vector<Type>::iterator;
	iter add(Type obj)
	{
		return data.emplace(data.end(), obj);
	}

	iter add(const std::vector<Type>& objs)
	{
		return data.insert(data.end(), objs.begin(), objs.end());
	}

	iter remove(Type* obj)
	{
		for (auto dataIt = data.begin(); dataIt != data.end(); ++dataIt)
		{
			if (&*dataIt == obj)
			{
				return data.erase(dataIt);
			}
		}
	}

//
	std::vector<Type> data;
};

class SimulationArea;
class ObjectManager
{
public:
	// temporary
	friend class SimulationArea;
	friend class RoadCreator;
	friend class BuildingCreator;
	friend class ObjectManagerUI;

	enum class CreatorType
	{
		ROAD,
		//BUILDING,
		MAX
	};

	ObjectManager(SimulationArea* pSimulationArea);
	~ObjectManager();

	void initialize();
	void update();

	void setCreatorsModes(Creator::CreatorMode mode);
	void setCurrentCreator(CreatorType creatorType);

	void drawUI();
//
private:
	SimulationArea* m_pSimulationArea;
	CreatorType m_currentCreator;
	Creator::CreatorMode m_currentCreatorMode;

	//
	Container<Road> m_roads;
	Container<RoadIntersection> m_intersections;
	Container<CarSpawner> m_carSpawners;
	Container<House> m_houses;

	RoadCreator m_roadCreator;
	//BuildingCreator m_buildingCreator;


public:
	void disableCreators();
	void enableCreators();

private: 
	bool m_disabledCreators = false;
};

