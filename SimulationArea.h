#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <utility>

#include <glm/glm.hpp>
#include "GraphicsComponent.h"
#include "SimulationAreaObject.h"

#include "RoadManager.h"
#include "RoadCreator.h"

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

	struct SimulationAreaData
	{
		std::vector<SimulationAreaObject*> objects;


		//std::vector<SimulationAreaObject*> buildings;
		//std::vector<SimulationAreaObject*> roads;
	};
}
namespace SAS = SimulationAreaStructs;

// illusiun at its best
class VulkanBase;
class SimulationAreaVisualizer
{
public:
	SimulationAreaVisualizer();
	void createVisuals(size_t xCount, size_t zCount, double distanceBetweenPoints);
	void update();
private:
	glm::vec3 position;
	GraphicsComponent graphics;
};

class SimulationArea final
{
	friend SimulationAreaVisualizer;
public:
	// with default values
	SimulationArea();
	~SimulationArea();
	void initArea();

	void loadData();

	void update();
	bool placeObject();
	bool placeSelectedObject();
	bool isInArea(const glm::vec3& position) const;

	void setEnableMouse(bool value);
	void clickEvent();

	std::pair<size_t, size_t> getPointsCount() const;
	std::optional<glm::vec3> getSelectedPointPos() const;
	std::optional<glm::vec3> getMousePosition() const;

	//[Obsolete("Should be refactored")]
	//SimulationAreaObjectCreator m_creator;

	RoadManager m_roadManager;
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
};

