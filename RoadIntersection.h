#pragma once
#include "SegmentedShape.h"
#include "BasicRoad.h"
#include "BasicGeometry.h"

#include <vector>
#include <array>
#include <set>


class Road;
class RoadIntersection
	: public BasicRoad
{
public:
	friend class RoadInspectorUI;

	void construct(std::array<Road*, 3> roads, Point intersectionPoint);
	void connectNewRoad(Road* newRoad);
	void setUpShape();
	void checkShapesAndRebuildIfNeeded();
	std::vector<Road*> disassemble();

	glm::vec3 getDirectionPointFromConnectionPoint(Point point) override;
	ConnectionPossibility getConnectionPossibility(LineSegment connectionLine, Shape::AxisPoint connectionPoint) const override;

	virtual void destroy() override;
	virtual bool hasBody() const override;
	bool sitsPointOn(Point point) const override;
	virtual RoadType getRoadType() const override;
	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const override;
	virtual void createLanes() override;
	virtual bool canSwitchLanes() const override;

	uint32_t directionCount() const;

protected:
	virtual Mesh createLineMesh() override;

private:

	float m_width = 0;
	Point m_centre = {};
	std::vector<SegmentedShape> m_connectShapes;
	std::vector<Point> m_outlinePoints;
};

