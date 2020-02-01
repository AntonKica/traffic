#pragma once
#include "Utilities.h"
#include "BasicRoad.h"
#include "SegmentedShape.h"

#include "RoadIntersection.h"
#include <vector>

class BasicBuilding;
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
	// overrided
	ConnectionPossibility getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const override;

	glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint) override;

	virtual void destroy() override;
	virtual bool hasBody() const override;
	bool sitsPointOn(Point point) const override;
	virtual RoadType getRoadType() const override;
	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const override;

	virtual void createPaths() override;
	virtual bool canSwitchLanes() const override;

	// Getters
	RoadParameters getParameters() const;
	bool sitsOnEndPoints(const Point& point) const;
	Shape::Axis getAxis() const;
	Points getAxisPoints() const;
	Point getCircumreferencePoint(Point roadPoint) const;
	SegmentedShape getShape() const;

	//
	void construct(Shape::Axis axisPoints, uint32_t laneCount, float width, std::string texture);
	void construct(Points creationPoints, uint32_t laneCount, float width, std::string texture);
	void construct(Points creationPoints, const RoadParameters& parameters);
	void reconstruct();

	void mergeWith(Road& otherRoad);
	struct SplitProduct;
	SplitProduct split(Shape::AxisPoint splitPoint);
	Road cutKnot();
	Road shorten(Shape::AxisPoint roadEnd, float size);
	void extend(Shape::AxisPoint roadEnd, Point point);
	SegmentedShape::ShapeCut getCut(Shape::AxisPoint roadAxisPoint) const;
	using CutProduct = SplitProduct;
	CutProduct cut(SegmentedShape::ShapeCut cutPoints);

	void resetNearbyBuildings();
	void addNearbyByuilding(BasicBuilding* nearbyBuilding, Point entryPoint);

	struct NearbyBuildingPlacement;
	const std::vector<NearbyBuildingPlacement>& getNearbyBuildings() const;

protected:
	virtual void newConnecionAction() override;
	virtual void lostConnectionAction() override;

private:
	SegmentedShape m_shape;
	RoadParameters m_parameters;

	struct NearbyBuildingPlacement
	{
		BasicBuilding* building;
		Point entryPoint;
	};

	std::vector<NearbyBuildingPlacement> m_nearbyBuildings;
	// just for cause
};

struct Road::SplitProduct
{
	std::optional<Road> road;

	//std::optional<Connection> connection;
};


