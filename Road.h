#pragma once
#include "Utilities.h"
#include "BasicRoad.h"
#include "RoadIntersection.h"
#include <vector>

bool polygonPointCollision(const Points& polygon, const Point& point);
//bool polygonPointCollision(const Points& vertices, float px, float py);
bool polygonPolygonCollision(const Points& polygonOne, const Points& polygonTwo);
namespace Shape
{
	struct AxisPoint : Point {
		AxisPoint() = default;
		AxisPoint(Point p) 
			: Point(p)
		{}
	};
	using Axis = std::vector<AxisPoint>;
	using AxisSegment = std::array<AxisPoint, 2>;
}

class SegmentedShape
{
public:
	struct Joint
	{
		Point left;
		Shape::AxisPoint centre;
		Point right;

		Joint() = default;
		Joint(Point left_, Shape::AxisPoint centre_, Point right_)
			: left(left_), centre(centre_), right(right_)
		{}
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

	std::optional<Segment> selectSegment(Point arbitraryPoint) const;
	std::vector<Segment> getJointSegments(Shape::AxisPoint jointPoint) const;
	std::optional<Shape::AxisPoint> getShapeAxisPoint(Point arbitraryPoint) const;

	static Mesh createMesh(const SegmentedShape& shape);
	// getters
	const Joints& getShape() const;
	Points getSkeleton() const;
	Shape::Axis getAxis() const;
	Shape::AxisPoint getHead() const;
	Shape::AxisPoint getTail() const;
	bool sitsOnHead(const Point& point) const;
	bool sitsOnTail(const Point& point) const;
	bool sitsOnTailOrHead(const Point& point) const;
	bool sitsOnShape(const Point& point) const;
	bool sitsOnAxis(const Point& point) const;
	bool sitsOnAnySegmentCorner(const Point& point) const;
	bool isCirculary() const;

	void reverseBody();

	// Connections
	// Tail
	void setTailDirectionPoint(Point point);
	void removeTailDirectionPoint();
	Point getTailDirectionPoint() const;
	bool hasTailDirectionPoint() const;
	// Head
	void setHeadDirectionPoint(Point point);
	void removeHeadDirectionPoint();
	Point getHeadDirectionPoint() const;
	bool hasHeadDirectionPoint() const;

	void clearDirectionPoints();



	// Creation and configration
	struct OrientedConstructionPoints
	{
		std::optional<Point> tailDirectionPoint;
		Points points;
		std::optional<Point> headDirectionPoint;
	};
	void construct(const Shape::Axis& axisPoints, float width);
	void construct(const Points& constructionPoints, float width);
	void construct(const OrientedConstructionPoints& constructionPoints, float width);
	void reconstruct();
	void mergeWith(const SegmentedShape& otherShape);
	std::optional<SegmentedShape> split(const Point& splitPoint);
	Shape::AxisPoint shorten(Shape::AxisPoint shapeEnd, float size);

	struct ShapeCut
	{
		Shape::Axis axis;
	};
	ShapeCut getShapeCut(Shape::AxisPoint axisPoint, float radius) const;
	Shape::AxisSegment getEdgesOfAxisPoint(Shape::AxisPoint axisPoint) const;

private:
	void setNewCircularEndPoints(Shape::AxisPoint axisPoint);
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
	void construct(Shape::Axis axisPoints, uint32_t laneCount, float width, std::string texture);
	void construct(Points creationPoints, uint32_t laneCount, float width, std::string texture);
	void construct(Points creationPoints, const RoadParameters& parameters);
	void reconstruct();

	void mergeWith(Road& otherRoad);
	struct SplitProduct;
	SplitProduct split(const Point& splitPoint);
	Point shorten(const Point& roadEnd, float size);
	Shape::Axis getCut(Point roadAxisPoint) const;
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

	std::optional<Connection> connection;
};


