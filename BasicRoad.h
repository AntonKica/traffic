#pragma once
#include "SimulationAreaObject.h"
#include "Utilities.h"
#include <map>
#include <glm/glm.hpp>

namespace Shape
{
	struct AxisPoint : Point {
		AxisPoint()										= default;
		AxisPoint(const AxisPoint& other)				= default;
		AxisPoint(AxisPoint&& other)					= default;
		AxisPoint& operator=(const AxisPoint& other)	= default;
		AxisPoint& operator=(AxisPoint&& other)			= default;

		explicit AxisPoint(const Point& p)
			: Point(p)
		{}

	};

	using Axis = std::vector<AxisPoint>;
	using AxisSegment = std::array<AxisPoint, 2>;
}

class BasicRoad :
	public SimulationAreaObject
{
public:
	BasicRoad();
	virtual ~BasicRoad();

	BasicRoad(const BasicRoad& copy);
	BasicRoad(BasicRoad&& move);
	BasicRoad& operator=(const BasicRoad& copy);
	BasicRoad& operator=(BasicRoad&& move);

	virtual glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint) = 0;
	struct ConnectionPossibility 
	{
		bool canConnect = false;
		Point recomendedPoint;

		inline operator bool() const
		{
			return canConnect;
		}
	};
	virtual ConnectionPossibility getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const = 0;

	enum class RoadType
	{
		ROAD,
		INTERSECTION,
		MAX_ROAD_TYPE
	};
	virtual void destroy() = 0;
	virtual bool hasBody() const = 0;
	virtual bool sitsPointOn(Point point) const = 0;
	virtual RoadType getRoadType() const = 0;

	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const = 0;
protected:
	struct Connection
	{
		BasicRoad* connected = nullptr;
		Point point = {};

		bool operator==(const BasicRoad* otherRoad) const
		{
			return this->connected == otherRoad;
		}
		bool operator==(const Point& otherPoint) const
		{
			return approxSamePoints(this->point, otherPoint);
		}
		bool operator==(const Connection& otherPair) const
		{
			return operator==(otherPair.connected) && operator==(otherPair.point);
		}
	};

	//Connection& findConnection(BasicRoad* connectedRoad);
	Connection& getConnection(Connection connection);
	Connection& getConnection(BasicRoad* road, Point point);
	/*
	* Carefully with this
	*/
	const Connection& getConnection(Point point) const;
	void connect(BasicRoad* connectionRoad, Point connectionPoint);
	void addConnection(Connection connection);
	void copyConnections(BasicRoad* destinationRoad) const;
	void transferConnections(BasicRoad* destinationRoad);
	void dismissConnection(Connection connection);
	void disconnectAll();

	std::vector<Connection> m_connections;
};

