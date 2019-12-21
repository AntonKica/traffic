#pragma once
#include "BuildingCreator.h"
#include "RoadCreator.h"
#include "UI.h"
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

class ObjectManager;
class ObjectManagerUI :
	public UIElement
{
public:
	ObjectManagerUI(ObjectManager* objectManager);

	virtual void draw() override;

	enum class Creator
	{
		BUILDING,
		ROAD,
		MAX
	};

private:
	Creator m_currentCreator;
	ObjectManager* m_pObjectManager;
};

class SimulationArea;
class ObjectManager
{
	// temporary
	friend class SimulationArea;
	friend class RoadCreator;
	friend class BuildingCreator;

public:
	ObjectManager(SimulationArea* pSimulationArea);
	void update();

	void updateSelectedRoad();
	std::optional<Road*> getSelectedRoad() const;
//
private:
	SimulationArea* m_pSimulationArea;

	ObjectManagerUI m_ui;

	//
	friend class UI;
	std::optional<Road*> m_selectedRoad;
	Container<Road> m_roads;
	RoadCreator m_roadCreator;
	//
	BuildingCreator m_buildingCreator;
};

