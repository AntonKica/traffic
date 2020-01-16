#pragma once
#include "BuildingCreator.h"
#include "RoadCreator.h"
#include "House.h"
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

class ObjectManager;
class ObjectManagerUI :
	public UIElement
{
public:
	enum class CreatorType
	{
		ROAD,
		BUILDING,
		MAX
	};
	ObjectManagerUI(ObjectManager* objectManager, CreatorType activeCreator);

	virtual void draw() override;

	CreatorType getCurrentCreator() const;
private:
	bool m_shouldCreate = true;
	CreatorType m_currentCreator;
	ObjectManager* m_pObjectManager;
};

class SimulationArea;
class ObjectManager
{
	// temporary
	friend class SimulationArea;
	friend class RoadCreator;
	friend class BuildingCreator;
	friend class ObjectManagerUI;
public:
	ObjectManager(SimulationArea* pSimulationArea);
	~ObjectManager();

	void initialize();
	void update();

	void updateSelectedRoad();
	std::optional<BasicRoad*> getSelectedRoad() const;
	void setCreatorsModes(Creator::CreatorMode mode);

	void clickEvent();
//
private:
	SimulationArea* m_pSimulationArea;
	ObjectManagerUI m_ui;

	//
	friend class UI;
	std::optional<BasicRoad*> m_selectedRoad;
	Container<Road> m_roads;
	Container<RoadIntersection> m_intersections;
	Container<House> m_houses;

	RoadCreator m_roadCreator;
	BuildingCreator m_buildingCreator;


public:
	void disableCreators();
	void enableCreators();

private: 
	bool m_disabledCreators = false;
};

