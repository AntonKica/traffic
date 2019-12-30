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
	void setDraw(const std::vector<Point>& drawAxis, const std::vector<Point>& drawPoints, float width, bool valid);

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
		std::optional<Road*> road;
	};
	using SittingPoints = std::vector<SittingPoint>;

	using PointRoadPair = std::pair<Point, Road*>;
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
	void validateCurrentShape();
	void handleCurrentPoints();
	void tryToConstructRoad();
	void tryToDestroyRoad();
	void createRoadFromCurrent();
	void handleConstruction(Road road, std::vector<RC::PointRoadPair> connectPoints);

	/*
	};*/
	// returns road you should use next
	Road* connectRoads(Road* road, Road* connectingRoad);
	// helper functions
	uint32_t connectCount(const Road& road, const Road& connectingRoad) const;
	std::vector<Shape::AxisPoint> connectPoints(const Road& road, const Road& connectingRoad) const;

	void mergeRoads(Road* road, Road* mergingRoad);
	Road* cutKnot(Road& road);
	void buildToIntersection(Road* road, Road* connectingRoad);

	ObjectManager* m_pRoadManager;

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
	//bool m_handleCurrentPointsNextUpdate = false;
protected:
	virtual void setCreatorModeAction() override;
	virtual void setActiveAction() override;

public:
	RoadCreator(ObjectManager* roadManager);


	void update();
	void clickEvent();
	void rollBackEvent();

	// temp function
};

