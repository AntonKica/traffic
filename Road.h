#pragma once
#include "Utilities.h"
#include "BasicRoad.h"
#include <vector>

bool polygonPointCollision(const Points& polygon, const Point& point);
bool polygonPointCollision(const Points& vertices, float px, float py);
bool polygonPolygonCollision(const Points& polygonOne, const Points& polygonTwo);

using Point = glm::vec3;
using Points = std::vector<Point>;

class SegmentedShape
{
public:
	struct Side
	{
		Point left, right;
	};
	using Sides = std::vector<Side>;
	struct Joint
	{
		Side side;
		Point centre;
	};
	using Joints = std::vector<Joint>;
	struct Segment
	{
		const Joint* start;
		const Joint* end;
	};

	std::optional<Segment> selectSegment(const Point& point) const;
	std::vector<Segment> getJointSegments(const Point& jointPoint) const;
	std::optional<Point> getShapeAxisPoint(const Point& point) const;

	static Mesh createMesh(const SegmentedShape& shape);
	// getters
	const Joints& getShape() const;
	Points getSkeleton() const;
	Points getAxis() const;
	Point getHead() const;
	Point getTail() const;
	bool sitsOnHead(const Point& point) const;
	bool sitsOnTail(const Point& point) const;
	bool sitsOnTailOrHead(const Point& point) const;
	bool sitsOnShape(const Point& point) const;
	bool sitsOnAnySegmentCorner(const Point& point) const;
	bool isCirculary() const;

	void reverseBody();

	//
	void construct(const Points& axis, float width);
	void megeWith(const SegmentedShape& otherShape);
	std::optional<SegmentedShape> split(const Point& splitPoint);
	Point shorten(const Point& shapeEnd, float size);
private:
	void setNewCircularEndPoints(const Point& point);
	void eraseCommonPoints();
	void createShape(const Points& axis);

	float m_width;
	Joints m_joints;
};

class Road :
	public BasicRoad
	//public SegmentedShape
{
public:
	struct RoadParameters
	{
		uint32_t laneCount;
		float width;
		std::string texture;
	};
public:
	// Getters
	RoadParameters getParameters() const;
	bool sitsOnEndPoints(const Point& point) const;
	bool sitsOnRoad(const Point& point) const;
	Point getPointOnRoad(const Point& point);

	//
	void construct(Points axisPoints, uint32_t laneCount, float width, std::string texture);
	void mergeWithRoad(const Road& road);
	std::optional<Road> split(const Point& splitPoint);
	Point shorten(const Point& roadEnd, float size);

	std::optional<Point> canConnect(std::array<Point, 2> connectionLine, const Point& connectionPoint) const;
private:
	void reconstruct();
	using Path = Points;
	void createPaths();

	SegmentedShape m_shape;
	RoadParameters m_parameters;
	// just for cause
public:
	std::vector<Path> m_paths;
};


