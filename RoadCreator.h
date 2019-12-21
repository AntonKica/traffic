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


namespace
{
	struct Prototypes
	{
		std::string name;
		uint32_t laneCount;
		float width;
		std::string texture;
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

	struct ConnectProducts
	{
		std::vector<Road> roads;
		std::vector<RoadIntersection> intersections;
		//std::vector<
	};

	void connectRoads(const Road& road, Road& connectingRoad);
	uint32_t connectCount(const Road& road, const Road& connectingRoad) const;
	std::vector<Point> connectPoints(const Road& road, const Road& connectingRoad) const;

	void mergeRoads(Road& road, const Road& mergingRoad);
	Road splitKnot(Road& road);
	void buildToIntersection(const Road& road, Road& connectingRoad);

	void updatePoints();
	enum class Mode
	{
		STRAIGHT_LINE,
		CURVED_LINE
	};
	ObjectManager* m_pRoadManager;

	Mode creatorMode{};
	CreatorVisualizer visualizer;

	struct SittingPoint
	{
		Point point = {};
		Road* road = nullptr;
		bool core = false;
	};
	std::vector<SittingPoint> setPoints;
	std::vector<SittingPoint> buildPoints;
	std::optional<SittingPoint> mousePoint;

	std::map<int, ::Prototypes> hardcodedRoadPrototypes;
	int currentPrototypeID = 0;
	bool validRoad = false;
public:
	RoadCreator(ObjectManager* roadManager);
	void update();
	void clickEvent();
	void rollBackEvent();

	// temp function
	std::vector<std::string> getRoadNames() const;
	void setMode(int mode);
	void setPrototype(int prototype);
};

