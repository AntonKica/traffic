#pragma once
#include "SimulationAreaObject.h"
#include "Utilities.h"
#include <map>
#include <glm/glm.hpp>

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
		std::optional<Point> recomendedPoint;

		operator bool() const
		{
			return canConnect;
		}
	};
	virtual ConnectionPossibility canConnect(Line connectionLine, Point connectionPoint) const = 0;

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
	Connection& findConnection(Point connectedPoint);
	void connect(BasicRoad* connectionRoad, Point connectionPoint);
	void addConnection(Connection connection);
	void copyConnections(BasicRoad* destinationRoad) const;
	void transferConnections(BasicRoad* destinationRoad);
	void dismissConnection(Connection& connection);
	void disconnect(Point connectedPoint);
	void disconnectAll();

	std::vector<Connection> m_connections;
};

