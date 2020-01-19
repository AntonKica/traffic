#pragma once
#include "Utilities.h"
#include "BasicRoad.h"
#include "RoadIntersection.h"
#include <vector>

class SegmentedShape
{
public:
	struct Joint
	{
		Point left;
		Shape::AxisPoint centre;
		Point right;

		Joint() = default;
		explicit Joint(const Point& left_,const Shape::AxisPoint& centre_, const Point& right_)
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
	std::optional<Shape::AxisPoint> getShapeAxisPoint(Point arbitraryPoint) const;

	static Mesh createMesh(const SegmentedShape& shape);
	// getters
	const Joints& getShape() const;
	Points getSkeleton() const;
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
	void mergeWith(const SegmentedShape& otherShape);
	std::optional<SegmentedShape> split(Shape::AxisPoint splitPoint);
	Shape::AxisPoint shorten(Shape::AxisPoint shapeEnd, float size);
	void extend(Shape::AxisPoint shapeEnd, Point point);

	struct ShapeCut
	{
		Shape::Axis axis;
	};
	ShapeCut getShapeCut(Shape::AxisPoint axisPoint, float radius) const;
	Shape::AxisSegment getEdgesOfAxisPoint(Shape::AxisPoint axisPoint) const;
	std::optional<SegmentedShape> cut(ShapeCut cutPoints);

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
	Shape::Axis getAxis() const;
	Points getAxisPoints() const;

	//
	void construct(Shape::Axis axisPoints, uint32_t laneCount, float width, std::string texture);
	void construct(Points creationPoints, uint32_t laneCount, float width, std::string texture);
	void construct(Points creationPoints, const RoadParameters& parameters);
	void reconstruct();

	void mergeWith(Road& otherRoad);
	struct SplitProduct;
	SplitProduct split(Shape::AxisPoint splitPoint);
	Shape::AxisPoint shorten(Shape::AxisPoint roadEnd, float size);
	void extend(Shape::AxisPoint roadEnd, Point point);
	SegmentedShape::ShapeCut getCut(Shape::AxisPoint roadAxisPoint) const;
	using CutProduct = SplitProduct;
	CutProduct cut(SegmentedShape::ShapeCut cutPoints);

	// overrided
	ConnectionPossibility getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const override;

	glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint) override;

	virtual void destroy() override;
	virtual bool hasBody() const override;
	bool sitsPointOn(Point point) const override;
	virtual RoadType getRoadType() const override;
	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const override;

	virtual void createPaths() override;
private:

	SegmentedShape m_shape;
	RoadParameters m_parameters;
	// just for cause
};

struct Road::SplitProduct
{
	std::optional<Road> road;

	//std::optional<Connection> connection;
};


