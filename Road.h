#pragma once
#include "Utilities.h"
#include "BasicRoad.h"
#include "RoadIntersection.h"
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
	enum SegmentPart
	{
		TAIL,
		HEAD
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
	bool sitsOnAxis(const Point& point) const;
	bool sitsOnAnySegmentCorner(const Point& point) const;
	bool isCirculary() const;

	void reverseBody();

	// Connections
	void setTailDirectionPoint(Point point);
	void removeTailDirectionPoint();
	Point getTailDirectionPoint() const;
	bool hasTailDirectionPoint() const;
	//
	void setHeadDirectionPoint(Point point);
	void removeHeadDirectionPoint();
	Point getHeadDirectionPoint() const;
	bool hasHeadDirectionPoint() const;

	void clearDirectionPoints();

	//
	struct OrientedConstructionPoints
	{
		std::optional<Point> tailDirectionPoint;
		Points points;
		std::optional<Point> headDirectionPoint;
	};
	void construct(const Points& axisPoints, float width);
	void construct(const OrientedConstructionPoints& constructionPoints, float width);
	void reconstruct();
	void mergeWith(const SegmentedShape& otherShape);
	std::optional<SegmentedShape> split(const Point& splitPoint);
	Point shorten(const Point& shapeEnd, float size);

private:
	void setNewCircularEndPoints(const Point& point);
	void eraseCommonPoints();
	void createShape(const Points& axis);

	float m_width;
	Joints m_joints;
	std::map<SegmentPart, Point> m_endDirectionPoints;
};

class Road :
	public BasicRoad
	//public SegmentedShape
{
public:
	friend class RoadCreator;

	struct RoadParameters
	{
		uint32_t laneCount = 0;
		float width = 0;
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
	void construct(Points axisPoints, const RoadParameters& parameters);
	void reconstruct();
	// fancy function
	void destruct();

	void mergeWith(Road& otherRoad);
	struct SplitProduct;
	SplitProduct split(const Point& splitPoint);
	Point shorten(const Point& roadEnd, float size);

	// overrided
	ConnectionPossibility canConnect(Line connectionLine, Point connectionPoint) const override;

	glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint) override;

private:
	using Path = Points;
	void createPaths();

	SegmentedShape m_shape;
	RoadParameters m_parameters;
	// just for cause
public:
	std::vector<Path> m_paths;
};

struct Road::SplitProduct
{
	std::optional<Road> road;
	RoadPointPair connection{};
};


