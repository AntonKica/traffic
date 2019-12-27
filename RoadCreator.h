#pragma once
#include "GraphicsComponent.h"
#include "Road.h"

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <set>
#include <optional>

using Point = glm::vec3;
using Points = std::vector<glm::vec3>;

template<class vec1> class VComparator
{
public:
	//static_assert(std::is_same < vec1>::value && "Not same vec");

	template <class T>
	T* ptr(T& obj) const { return &obj; }
	template <class T>
	T* ptr(T* obj) const { return obj; }

	bool operator()(const vec1& v1, const vec1& v2) const
	{
		return std::memcmp(ptr(v1), ptr(v2), sizeof(*ptr(v1))) == 1;
	}
};

class CreatorVisualizer
{
public:
	void update();
	void setDraw(const std::vector<Point>& points, float width, bool valid);

private:
	VD::PositionVertices generateLines();
	std::pair<VD::PositionVertices, VD::ColorVertices> generatePoints();
	void updateGraphics();

	float width = 0;
	bool valid = false;
	std::vector<Point> pointToDraw;
	std::optional<Point> mousePoint;

	GraphicsComponent pointGraphics;
	GraphicsComponent lineGraphics;
};


namespace RC
{
	struct Prototypes
	{
		std::string name;
		uint32_t laneCount;
		float width;
		std::string texture;
	};

	struct SittingPoint
	{
		struct SittingAxisPoint
		{
			Shape::AxisPoint axisPoint;
			Road* road = nullptr;
		};
		using SAP = SittingAxisPoint;

		std::variant<Point, SAP> point;

		Point getPoint() const
		{
			if (auto point = std::get_if<Point>(&this->point))
				return *point;
			else
				return std::get<RC::SittingPoint::SAP>(this->point).axisPoint;
		}
	};
}

class ObjectManager;
class RoadCreator
{
private:
	void setupPrototypes();
	void setPoint();
	void createRoadIfPossible();
	void createRoad(const Points& creationPoints);
	void handleConstruction(Road road, std::vector<RC::SittingPoint::SAP> connectPoints);

	void deleteRoadIfPossible();

	/*
	struct ConnectProducts
	{
		std::vector<Road> roads;
		std::vector<RoadIntersection> intersections;
		//std::vector<
	};*/
	// returns road you should use next
	Road* connectRoads(Road* road, Road* connectingRoad);
	uint32_t connectCount(const Road& road, const Road& connectingRoad) const;
	std::vector<Shape::AxisPoint> connectPoints(const Road& road, const Road& connectingRoad) const;

	void mergeRoads(Road* road, Road* mergingRoad);
	Road* cutKnot(Road& road);
	void buildToIntersection(Road* road, Road* connectingRoad);

	void updatePoints();
	enum class CreateMode
	{
		CREATOR,
		DELETOR
	};
	enum class BuildMode
	{
		STRAIGHT_LINE,
		CURVED_LINE
	};
	ObjectManager* m_pRoadManager;

	BuildMode buildMode{};
	CreateMode createMode{};
	CreatorVisualizer visualizer;

	std::vector<RC::SittingPoint> setPoints;
	std::vector<RC::SittingPoint> buildPoints;
	std::optional<RC::SittingPoint> mousePoint;

	std::map<int, RC::Prototypes> hardcodedRoadPrototypes;
	int currentPrototypeID = 0;
	bool validRoad = false;
public:
	RoadCreator(ObjectManager* roadManager);

	void update();
	void clickEvent();
	void rollBackEvent();

	// temp function
	std::vector<std::string> getRoadNames() const;
	void setBuildMode(int mode);
	void setCreateMode(int mode);
	void setPrototype(int prototype);
};

