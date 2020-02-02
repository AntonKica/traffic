#pragma once
#include "BasicRoad.h"

class Road;
class CarSpawner :
	public BasicRoad
{
public:
	friend class RoadInspectorUI;

	virtual void update() override;

	virtual glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint);
	virtual ConnectionPossibility getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const;

	virtual void destroy();
	virtual bool hasBody() const;
	virtual bool sitsPointOn(Point point) const;
	virtual RoadType getRoadType() const;

	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const;

	virtual void createPaths();
	virtual bool canSwitchLanes() const;

	void construct(Road* road, Point connectPoint);

	void disable();
	void enable();
private:
	Points m_shapePoints;
	bool m_disabled;
};

