#pragma once
#include "SimulationObject.h"
#include "SegmentedShape.h"

#include "Utilities.h"
#include <map>
#include <glm/glm.hpp>

class BasicRoad;
struct Path
{
	Points points;
	BasicRoad* connectsTo = nullptr;
	BasicRoad* connectsFrom = nullptr;
	enum class Side { LEFT, RIGHT };
	Side side = {};

	static bool pathLeadsToPath(const Path& path, const Path& leadsToPath);
	friend bool operator==(const Path& lhs, const Path& rhs);
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
	virtual ConnectionPossibility getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const = 0;

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

	virtual void createPaths() = 0;
	virtual bool canSwitchLanes() const = 0;
	Path getClosestPath(Point pt) const;
	std::vector<Path> getSubsequentPathsFromConnectingPath(const Path& connectingPath) const;
	std::vector<Path> getAllPathsConnectingTo(const BasicRoad* const connectsToRoad) const;
	std::vector<Path> getAllPathsConnectingTwoRoads(const BasicRoad* const connectsFromRoad, const BasicRoad* const connectsToRoad) const;
protected:

	//Connection& findConnection(BasicRoad* connectedRoad);
	const Connection& getConnection(Connection connection) const;
	const Connection& getConnection(BasicRoad* road, Point point) const;
	/*
	* Carefully with this
	*/
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
	std::unordered_map<Path::Side, std::vector<Path>> m_paths;
private:
};

