#pragma once
#include "BasicRoad.h"
#include "SegmentedShape.h"

#include <vector>
#include <optional>

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

		struct
		{
			float laneWidth = Defaults::laneWidth;
			float lineWidth = Defaults::lineWidth;

			float distanceFromSide = Defaults::distanceFromSide;
			float sideLineWidth = Defaults::sideLineWidth;
			std::optional<float> separatorWidth = Defaults::separatorWidth;
		} lineInfo;

		bool isOneWay = Defaults::isOneWay;
		bool forceOneWay = Defaults::forceOneWay;
	};
}

namespace Products
{
	struct Split;
	using Cut = Split;
}

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
	void construct(Points creationPoints, const RoadParameters::Parameters& parameters);
	void reconstruct();

	void mergeWith(Road& otherRoad);
	Road cutKnot();
	Road shorten(Shape::AxisPoint roadEnd, float size);
	void extend(Shape::AxisPoint roadEnd, Point point);
	// products
	Products::Split split(Shape::AxisPoint splitPoint);

	SegmentedShape::ShapeCut getCut(Shape::AxisPoint roadAxisPoint) const;
	Products::Cut cut(SegmentedShape::ShapeCut cutPoints);

	/*
	void resetNearbyBuildings();
	void addNearbyByuilding(BasicBuilding* nearbyBuilding, Point entryPoint);

	struct NearbyBuildingPlacement;
	const std::vector<NearbyBuildingPlacement>& getNearbyBuildings() const;*/
protected:
	virtual Mesh createLineMesh() override;

private:
	void updateWidthFromParameters();
	void updateLength();

	SegmentedShape m_shape;
	RoadParameters::Parameters m_parameters;
	float m_width = 0.0f;
	float m_length = 0.0f;
};

// declarations
namespace Products
{
	struct Split
	{
		std::optional<Road> road;
	};
}