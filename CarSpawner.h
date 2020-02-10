#pragma once
#include "BasicRoad.h"
#include "SegmentedShape.h"
#include "RoadPathFinder.h"
#include "SimpleCar.h"

class Road;
class CarSpawner :
	public BasicRoad
{
public:
	friend class RoadInspectorUI;

	virtual void update() override;

	virtual glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint);
	virtual ConnectionPossibility getConnectionPossibility(LineSegment connectionLine, Shape::AxisPoint connectionPoint) const;

	virtual void destroy();
	virtual bool hasBody() const;
	virtual bool sitsPointOn(Point point) const;
	virtual RoadType getRoadType() const;

	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const;

	virtual void createLanes();
	virtual bool canSwitchLanes() const;

	void construct(Road* road, Point connectPoint);

	void disable();
	void enable();

	void initializePossiblePaths(const std::vector<CarSpawner>& allSpawners);
	void spawnCar();

	bool canReceiveCars() const;
protected:
	virtual Mesh createLineMesh() override;


private:
	SegmentedShape m_shape;
	Points m_shapePoints;
	bool m_disabled;
	bool m_canSpawnCars = true;
	bool m_canReceiveCars = true;

	std::vector<SimpleCar> m_spawnedCars;

	std::unordered_map<const CarSpawner*, PathFinding::RoadRoutes> m_routesToSpawner;
};

