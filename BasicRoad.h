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

	virtual glm::vec3 getConnectionDirectionPoint(BasicRoad* road) = 0;

protected:
	struct RoadPointPair
	{
		BasicRoad* road = nullptr;
		Point point;

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
	static void dismissConnection(RoadPointPair& connection);
	static void disconnect(BasicRoad* road1, BasicRoad* road2);
	static void disconnectAll(BasicRoad* road);

	std::vector<RoadPointPair> m_connections;
};

