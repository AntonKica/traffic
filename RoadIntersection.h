#pragma once
#include "SegmentedShape.h"
#include "BasicRoad.h"
#include "Utilities.h"
#include <vector>
#include <array>
#include <set>


class Road;
class RoadIntersection
	: public BasicRoad
{
public:
	void construct(std::array<Road*, 3> roads, Point intersectionPoint);
	void setUpShape();
	bool validIntersection();
	std::vector<Road*> disassemble();

	glm::vec3 getDirectionPointFromConnectionPoint(Point point) override;
	ConnectionPossibility getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const override;

	virtual void destroy() override;
	virtual bool hasBody() const override;
	bool sitsPointOn(Point point) const override;
	virtual RoadType getRoadType() const override;
	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const override;
	virtual void createPaths() override;
	virtual bool canSwitchLanes() const override;

protected:
	virtual void newConnecionAction();
	virtual void lostConnectionAction();

private:
	float m_width = 0;
	Point m_centre = {};
	std::vector<SegmentedShape> m_connectShapes;
	std::vector<Point> m_outlinePoints;
	std::vector<Point> m_shapePoints;
};

