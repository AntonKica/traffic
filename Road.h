#pragma once
#include "Utilities.h"
#include "BasicRoad.h"
#include "SegmentedShape.h"

#include "RoadIntersection.h"
#include <vector>

namespace RoadParameters
{
	namespace Defaults
	{
		constexpr uint32_t laneCount = 1;
		constexpr float laneWidth = 3.0f;
		constexpr float lineWidth = 0.25f;

		constexpr float distanceFromSide = 0.1f;
		constexpr float sideLineWidth = 0.3f;
		const std::optional<float> separatorWidth;

		constexpr bool isOneWay = false;
		constexpr bool forceOneWay = false;
	}
	struct Parameters
	{
		uint32_t laneCount	= Defaults::laneCount;
		float laneWidth		= Defaults::laneWidth;
		float lineWidth		= Defaults::lineWidth;

		float distanceFromSide				= Defaults::distanceFromSide;
		float sideLineWidth					= Defaults::sideLineWidth;
		std::optional<float> separatorWidth	= Defaults::separatorWidth;
		bool isOneWay = Defaults::isOneWay;
		bool forceOneWay = Defaults::forceOneWay;
	};
}
class BasicBuilding;
class Road :
	public BasicRoad
{
public:
	friend class RoadInspectorUI;
	friend class RoadCreator;

public:
	// overrided
	ConnectionPossibility getConnectionPossibility(LineSegment connectionLine, Shape::AxisPoint connectionPoint) const override;

	glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint) override;

	virtual void destroy() override;
	virtual bool hasBody() const override;
	bool sitsPointOn(Point point) const override;
	virtual RoadType getRoadType() const override;
	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const override;

	virtual void createLanes() override;
	virtual bool canSwitchLanes() const override;

	// Getters
	RoadParameters::Parameters getParameters() const;
	float getWidth() const;
	float getLength() const;
	bool sitsOnEndPoints(const Point& point) const;
	Shape::Axis getAxis() const;
	Points getAxisPoints() const;
	Point getCircumreferencePoint(Point roadPoint) const;
	const SegmentedShape& getShape() const;
	Shape::AxisPoint getClosestEndPoint(Shape::AxisPoint roadPoint) const;
	glm::vec3 getDirectionFromEndPoint(Shape::AxisPoint endPoint) const;

	//
	void construct(Shape::Axis axisPoints, const RoadParameters::Parameters& parameters);
	void construct(Points creationPoints, const RoadParameters::Parameters& parameters);
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

private:
	void updateWidthFromParameters();
	void updateLength();

	SegmentedShape m_shape;
	RoadParameters::Parameters m_parameters;
	float m_width = 0.0f;
	float m_length = 0.0f;

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


