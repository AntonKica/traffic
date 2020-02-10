#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <utility>

#include <glm/glm.hpp>
#include "GraphicsComponent.h"
#include "SimulationObject.h"
#include "SimpleCar.h"
#include "ObjectManager.h"
#include "RoadInspector.h"

namespace Settings
{
	namespace SimulationArea
	{
		constexpr bool multipleOfTwo(int num)
		{
			return (num != 0) && (num & (num - 1)) == 0;
		}

		const glm::vec3 origin = glm::vec3(0.0);

		constexpr float unitLength = 1.0;
		constexpr int pointsPerUnit = 2;

		static_assert(pointsPerUnit > 1 && pointsPerUnit < 32 && multipleOfTwo(pointsPerUnit)
			&& "Tile per one unit > 1 and < 32 and is multiple of 2");

		constexpr double defaultTileSize = 1.0 / pointsPerUnit;
		constexpr int defaultSize = 500;
		static_assert(defaultSize <= 2000 &&
			"Wth, nejako vela");
	}
}
namespace SSA = Settings::SimulationArea;

namespace SimulationAreaStructs
{
	struct SimulationAreaTraits
	{
		glm::vec3 Position;

		float UnitLength;
		uint16_t PointsPerUnit;

		uint32_t AreaWidth;
		uint32_t AreaHeight;
	};
}
namespace SAS = SimulationAreaStructs;

// illusiun at its best
class SimulationArea;
class SimulationAreaVisualizer
{
public:
	SimulationAreaVisualizer(SimulationArea* pSimulationArea);

	void createVisuals(size_t xCount, size_t zCount, double distanceBetweenPoints);
	void update();
private:
	SimulationArea* m_pSimulationArea;

	GraphicsComponent graphics;
	glm::vec3 position;
};

class TopMenu : 
	public UIElement
{
public:
	TopMenu(SimulationArea* pSimulationArea);

	virtual void draw() override;

	bool pressedPlay() const;
private:
	SimulationArea* m_pSimulationArea;

	bool m_pressedPlay = false;
};

class BottomMenu :
	public UIElement
{
public:
	BottomMenu(SimulationArea* pSimulationArea);

	virtual void draw() override;

private:
	SimulationArea* m_pSimulationArea;
};

class SimulationArea final
{
	friend class SimulationAreaVisualizer;
public:
	enum class Mode
	{
		CREATE,
		DESTROY,
		INSPECT
	};

	enum class SimulationMode
	{
		RUN,
		EDIT
	};

	// with default values
	SimulationArea();
	~SimulationArea();

	void initArea();

	void loadData();

	void update();
	void handleCurrentMode();
	bool placeObject();
	bool isInArea(const glm::vec3& position) const;

	void setEnableMouse(bool value);
	void setMode(Mode mode);
	void setSimualtionMode(SimulationMode simulationMode);

	Mode getMode() const;
	SimulationMode getSimualtionMode() const;

	std::pair<size_t, size_t> getPointsCount() const;
	std::optional<glm::vec3> getSelectedPointPos() const;
	std::optional<glm::vec3> getMousePosition() const;

	void updateSelecteObject();
	std::optional<SimulationObject*> getSelectedObject() const;

	//[Obsolete("Should be refactored")]

	ObjectManager m_objectManager;
private:
	void initTraits();
	void updateMousePosition();
	inline glm::vec3 getNearestPoint(const glm::vec3& position) const;
	inline float getDirectPointDistance() const;


	SAS::SimulationAreaTraits m_traits;
	SimulationAreaVisualizer m_visuals;
	// where mouse falls on area
	
	std::optional<glm::vec3> m_mousePosition;
	bool m_enableMouse = false;
	std::optional<SimulationObject*> m_selectedObject;
	RoadInspector m_roadInspector;
	// new
	TopMenu m_topMenu;
	BottomMenu m_bottomMenu;
	Mode m_currentMode = {};
	SimulationMode m_currentSimulationMode;

	/*const Road* findClosestRoadFromBuilding(const BasicBuilding& building) const;
	void connectBuildingsAndRoads();
	void connectBuildingToClosestRoad(BasicBuilding& building);*/
	void runSimulation();
	void stopSimulation();
};

