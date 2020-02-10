#pragma once
#include "SimulationObject.h"
#include "SegmentedShape.h"
#include "Mesh.h"

#include <map>
#include <glm/glm.hpp>

class BasicRoad;
struct Lane
{
	Points points;
	BasicRoad* connectsTo = nullptr;
	BasicRoad* connectsFrom = nullptr;
	enum class Side { LEFT, RIGHT };
	Side side = {};

	bool leadsToLane(const Lane& leadsToLane) const;
	bool empty() const;
	friend bool operator==(const Lane& lhs, const Lane& rhs);
};

class BasicRoad :
	public SimulationObject
{
public:
	friend class RoadInspectorUI;

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

	BasicRoad();
	virtual ~BasicRoad();

	BasicRoad(const BasicRoad& copy);
	BasicRoad(BasicRoad&& move);
	BasicRoad& operator=(const BasicRoad& copy);
	BasicRoad& operator=(BasicRoad&& move);

	uint32_t getConnectedCount() const;
	const std::vector<Connection>& getConnections() const;
	bool canConnect(Point point) const;

	virtual glm::vec3 getDirectionPointFromConnectionPoint(Point connectionPoint) = 0;
	struct ConnectionPossibility 
	{
		bool canConnect = false;
		Point recomendedPoint = {};

		inline operator bool() const
		{
			return canConnect;
		}
	};
	virtual ConnectionPossibility getConnectionPossibility(LineSegment connectionLine, Shape::AxisPoint connectionPoint) const = 0;

	enum class RoadType
	{
		ROAD,
		INTERSECTION,
		CAR_SPAWNER,
		MAX_ROAD_TYPE
	};


	virtual void destroy() = 0;
	virtual bool hasBody() const = 0;
	virtual bool sitsPointOn(Point point) const = 0;
	virtual RoadType getRoadType() const = 0;

	virtual Shape::AxisPoint getAxisPoint(Point pointOnRoad) const = 0;

	virtual void createLanes() = 0;
	virtual bool canSwitchLanes() const = 0;
	Lane getClosestLane(Point pt) const;
	std::unordered_map<Lane::Side, std::vector<Lane>> getAllLanes()const;
	std::vector<Lane> getSubsequentLanesConnectingFromLane(const Lane& connectingLane) const;
	std::vector<Lane> getSubsequentLanesConnectingToLane(const Lane& connectingLane) const;
	std::vector<Lane> getAllLanesConnectingTo(const BasicRoad* const connectsToRoad) const;
	std::vector<Lane> getAllLanesConnectingFrom(const BasicRoad* const connectsFromRoad) const;
	std::vector<Lane> getAllLanesConnectingTwoRoads(const BasicRoad* const connectsFromRoad, const BasicRoad* const connectsToRoad) const;
protected:
	const Connection& getConnection(Connection connection) const;
	const Connection& getConnection(BasicRoad* road, Point point) const;

	const Connection& getConnection(Point point) const;
	const Connection* findConnection(Point point) const;
	BasicRoad* findConnectedRoad(Point point) const;

	void connect(BasicRoad* connectionRoad, Point connectionPoint);
	void addConnection(Connection connection);
	void copyConnections(BasicRoad* destinationRoad) const;
	void transferConnections(BasicRoad* destinationRoad);
	void dismissConnection(Connection connection);
	void disconnectAll();

	std::vector<Connection> m_connections;
	std::unordered_map<Lane::Side, std::vector<Lane>> m_lanes;

// lanes
	virtual Mesh createLineMesh() = 0;
private:
};

