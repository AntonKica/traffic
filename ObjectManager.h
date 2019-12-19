#pragma once
#include "BuildingCreator.h"
#include "RoadCreator.h"
#include <list>

template <class Type> class Container
{
public:
	void add(Type obj)
	{
		data.push_back(obj);
	}

	void add(const std::vector<Type>& objs)
	{
		data.insert(data.end(), objs.begin(), objs.end());
	}

	void remove(Type* obj)
	{
		for (auto dataIt = data.begin(); dataIt != data.end(); ++dataIt)
		{
			if (&*dataIt == obj)
			{
				data.erase(dataIt);
				break;
			}
		}
	}

	void remove(const std::vector<Type*>& objs)
	{
		for (auto& obj : objs)
			remove(obj);
	}

//
	std::list<Type> data;
};

class SimulationArea;
class ObjectManager
{
	friend class RoadCreator;
	friend class BuildingCreator;

public:
	ObjectManager(SimulationArea* pSimulationArea);
	void update();
//private:
	/*struct
	{

	} roads;*/
//roads

	void updateSelectedRoad();
	std::optional<Road*> getSelectedRoad() const;
//
private:
	SimulationArea* m_pSimulationArea;

	friend class UI;
	std::optional<Road*> m_selectedRoad;
	Container<Road> m_roads;
	RoadCreator m_roadCreator;
	//
	BuildingCreator m_buildingCreator;
};

