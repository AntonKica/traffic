#pragma once
#include "GraphicsComponent.h"
#include "Road.h"
#include "BasicCreator.h"

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <set>
#include <optional>

/*
using Point = glm::vec3;
using Points = std::vector<glm::vec3>;*/
/*
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
};*/

class CreatorVisualizer
{
public:
	CreatorVisualizer();
	~CreatorVisualizer();

	void update();
	void setDraw(const std::vector<Point>& drawAxis, const std::vector<Point>& drawPoints, float width, bool valid);
	void setActive(bool active);
private:
	VD::PositionVertices generateLines();
	std::pair<VD::PositionVertices, VD::ColorVertices> generatePoints();
	void updateGraphics();

	float width = 0;
	bool valid = false;
	std::vector<Point> axisToDraw;
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
		Point point;
		std::optional<BasicRoad*> road;
	};
	using SittingPoints = std::vector<SittingPoint>;

	using PointRoadPair = std::pair<Point, BasicRoad*>;
	struct ProcessedSittingPoints
	{
		bool validPoints = true;
		std::optional<PointRoadPair> roadBeginAxisPoint;
		Points axisPoints;
		std::optional<PointRoadPair> roadEndAxisPoint;
	};
	using ProcSitPts = ProcessedSittingPoints;
}

class RoadCreatorUI
	: public BasicUI
{
public:
	RoadCreatorUI();
	virtual void draw() override;

	bool doCurves() const;
	const RC::Prototypes* getSelectedPrototype() const;

private:
	bool m_doCurves = false;
	const RC::Prototypes* m_selectedPrototype;
	std::vector<RC::Prototypes> m_prototypes;
};

class ObjectManager;
class RoadCreator
	: public BasicCreator<RoadCreatorUI>
{
public:
	RoadCreator(ObjectManager* objectManager);

	void update();
	void rollBackEvent();

protected:
	virtual void setCreatorModeAction() override;
	virtual void setActiveAction() override;

private:
	void updatePoints();
	void updateMousePoint();
	void updateCurrentPoints();
	void updateCreationPoints();
	void setVisualizerDraw();
	struct ProcessedSittingPoints
	{
		std::optional<Point> roadBeginAxisPoint;
		Point constructPoints;
		std::optional<Point> roadEndAxisPoint;
	};
	RC::ProcSitPts processSittingPoints(const std::vector<RC::SittingPoint> sittingPoints) const;
	void setPoint();
	void constructRoadPrototype();
	void handleCurrentPoints();
	void tryToConstructRoad();
	void tryToDestroyRoad();
	void tidyIntersections();
	void createRoadFromCurrent();
	void handleConstruction(Road road, std::vector<RC::PointRoadPair> connectPoints);

	/*
	};*/
	// returns road you should use next
	struct ConnectProducts
	{
		std::vector<Road> newRoads;
		std::vector<RoadIntersection> newIntersections;
		bool keepConnectingRoad = true;
	};
	ConnectProducts connectRoads(Road& road, Road& connectingRoad);
	// helper functions
	uint32_t connectCount(const Road& road, const Road& connectingRoad) const;
	std::vector<Shape::AxisPoint> connectPoints(const Road& road, const Road& connectingRoad) const;

	void mergeRoads(Road& road, Road& mergingRoad);
	struct IntersectionProducts
	{
		std::vector<Road> newRoads;
		std::vector<RoadIntersection> newIntersections;
	};
	IntersectionProducts buildToIntersection(Road& road, Road& connectingRoad);

	CreatorVisualizer visualizer;

	std::optional<RC::SittingPoint> m_mousePoint;
	std::vector<RC::SittingPoint> m_setPoints;
	struct
	{
		bool frontOnRoad = false;
		bool backOnRoad = false;
		Points points;
	} m_creationPoints;

	RC::ProcSitPts m_processedCurrentPoints;
	bool m_currentShapeValid = false;
	Road m_roadPrototype;
	//bool m_handleCurrentPointsNextUpdate = false;
};

