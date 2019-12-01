#pragma once
#include "GraphicsComponent.h"
#include "Road.h"

#include <glm/glm.hpp>
#include <vector>
#include <map>

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
	void setDraw(const std::vector<Point>& points, float width);

private:
	std::vector<glm::vec3> generateLines();
	std::vector<glm::vec3> generatePoints();
	void updateGraphics();

	float width = 0;
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

class RoadManager;
class RoadCreator
{
private:
	void setupPrototypes();
	void setPoint();
	void createRoadIfPossible();
	void updatePoints();
	enum class Mode
	{
		STRAIGHT_LINE,
		CURVED_LINE
	};
	RoadManager* roadManager;

	Mode creatorMode{};
	CreatorVisualizer visualizer;

	struct SittingPoint
	{
		Point point = {};
		Road* road = nullptr;
	};
	std::vector<SittingPoint> currentPoints;
	std::vector<SittingPoint> placedPoints;

	std::map<int, ::Prototypes> hardcodedRoadPrototypes;
	int currentPrototypeID = 0;
public:
	void initialize(RoadManager* roadManager);
	void update();
	void clickEvent();
	void rollBackEvent();

	// temp function
	std::vector<std::string> getRoadNames() const;
	void setMode(int mode);
	void setPrototype(int prototype);
};

