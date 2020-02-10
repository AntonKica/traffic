#pragma once
#include "Utilities.h"
#include "Mesh.h"

#include <vector>

namespace Shape
{
	struct AxisPoint : Point {
		AxisPoint() = default;
		AxisPoint(const AxisPoint& other) = default;
		AxisPoint(AxisPoint&& other) = default;
		AxisPoint& operator=(const AxisPoint& other) = default;
		AxisPoint& operator=(AxisPoint&& other) = default;

		explicit AxisPoint(const Point& p)
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
		explicit Joint(const Point& left_, const Shape::AxisPoint& centre_, const Point& right_)
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
	std::vector<Segment> getSegments(Shape::AxisPoint axisPoint) const;
	Shape::AxisPoint getShapeAxisPoint(Point arbitraryPoint) const;
	Point getCircumreferencePoint(Point shapePoint) const;
	bool isHeadSegment(const Segment& segment) const;
	bool isTailSegment(const Segment& segment) const;

	static Mesh createMesh(const SegmentedShape& shape);
	// getters
	const Joints& getShape() const;
	Points getSkeleton() const;
	Points getOutline() const;
	Points getLeftSidePoints() const;
	Points getRightSidePoints() const;
	Shape::Axis getAxis() const;
	Points getAxisPoints() const;
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
private:
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
public:


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
	//
	void mergeWith(const SegmentedShape& otherShape);
	SegmentedShape cutKnot();
	SegmentedShape shorten(Shape::AxisPoint shapeEnd, float size);
	void extend(Shape::AxisPoint shapeEnd, Point point);
	//
	std::optional<SegmentedShape> split(Shape::AxisPoint splitPoint);
	//
	struct ShapeCut
	{
		Shape::Axis axis;
	};
	ShapeCut getShapeCut(Shape::AxisPoint axisPoint, float radius) const;
	std::optional<SegmentedShape> cut(ShapeCut cutPoints);
	Shape::AxisSegment getEdgesOfAxisPoint(Shape::AxisPoint axisPoint) const;

private:
	void setNewCircularEndPoints(Shape::AxisPoint axisPoint);
	void purifyPoints(Points& axis);
	void createShape(Points axis);

	float m_width;
	Joints m_joints;
	std::map<SegmentPart, Point> m_endDirectionPoints;
};

