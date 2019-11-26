#pragma once
#include "GraphicsComponent.h"

#include <glm/glm.hpp>
#include <vector>
#include <set>

using Point = glm::vec3;
class Road
{
private:
	glm::vec3 centralisePointsToPosition(std::vector<Point>& pts);
public:
	Road();

	void createGraphics(std::vector<Point> pts);

	std::vector<Point> roadPoints;
	GraphicsComponent graphics;
};

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
	void setPoints(const std::vector<Point>& points);

private:
	std::vector<glm::vec3> generateLines();
	std::vector<glm::vec3> generatePoints();
	void updateGraphics();

	std::vector<Point> pointToDraw;
	std::optional<Point> mousePoint;

	GraphicsComponent pointGraphics;
	GraphicsComponent lineGraphics;
};

class RoadCreator
{
private:
	CreatorVisualizer visualizer;
	using pointComparator = VComparator<Point>;
	std::vector<Point> currentPoints;
	std::vector<Road> tempRoads;

	void setPoint();
	void createRoadFromCurrent();
public:
	void update();
	void clickEvent();
};

