#pragma once
#include "SimulationAreaObject.h"
#include "Utilities.h"
#include <map>
#include <glm/glm.hpp>

class BasicRoad :
	public SimulationAreaObject
{
public:
	virtual ~BasicRoad();

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
	struct RoadPointPair
	{
		BasicRoad* road = nullptr;
		Point point = {};

		bool operator==(const BasicRoad* otherRoad) const
		{
			return this->road == otherRoad;
		}
		bool operator==(const Point& otherPoint) const
		{
			return approxSamePoints(this->point, otherPoint);
		}
		bool operator==(const RoadPointPair& otherPair) const
		{
			return operator==(otherPair.road) && operator==(otherPair.point);
		}
	};
	static RoadPointPair& findConnection(BasicRoad* road, BasicRoad* connectedRoad);
	static RoadPointPair& findConnection(BasicRoad* road, Point connectedPoint);
	static void connect(BasicRoad* road, const RoadPointPair& connection);
	static void connect(BasicRoad* road1, BasicRoad* road2, Point connectionPoint);
	static void transferConnections(BasicRoad* sourceRoad, BasicRoad* destinationRoad);
	static void dismissConnection(RoadPointPair& connection);
	static void disconnect(BasicRoad* road1, BasicRoad* road2);
	static void disconnectAll(BasicRoad* road);

	std::vector<RoadPointPair> m_connections;
};

