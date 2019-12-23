#include "BasicRoad.h"

BasicRoad::~BasicRoad()
{
	//for (auto& connection : m_connections)
	//	disconnect(this, connection.road);
}

BasicRoad::RoadPointPair& BasicRoad::findConnection(BasicRoad* road, BasicRoad* connectedRoad)
{
	return *std::find(std::begin(road->m_connections), std::end(road->m_connections), connectedRoad);
}

BasicRoad::RoadPointPair& BasicRoad::findConnection(BasicRoad* road, Point connectedPoint)
{
	return *std::find(std::begin(road->m_connections), std::end(road->m_connections), connectedPoint);
}

void BasicRoad::connect(BasicRoad* road, const RoadPointPair& connection)
{
	connect(road, connection.road, connection.point);
}

void BasicRoad::connect(BasicRoad* road1, BasicRoad* road2, Point connectionPoint)
{
	RoadPointPair connection;
	connection.point = connectionPoint;
	connection.road = road2;
	road1->m_connections.push_back(connection);

	connection.road = road1;
	road2->m_connections.push_back(connection);
}

void BasicRoad::transferConnections(BasicRoad* sourceRoad, BasicRoad* destinationRoad)
{
	for (auto& sourceConnections : sourceRoad->m_connections)
	{
		connect(destinationRoad, sourceConnections);
		dismissConnection(sourceConnections);
	}
}

void BasicRoad::dismissConnection(RoadPointPair& connection)
{
	auto& firstRoad = connection.road;
	auto& secondRoad = findConnection(firstRoad, connection.point).road;

	disconnect(firstRoad, secondRoad);
}

void BasicRoad::disconnect(BasicRoad* road1, BasicRoad* road2)
{
	auto& firstConnections = road1->m_connections;
	auto& secondConnections = road2->m_connections;

	// no control
	firstConnections.erase(std::find(std::begin(firstConnections), std::end(firstConnections), road2));
	secondConnections.erase(std::find(std::begin(secondConnections), std::end(secondConnections), road1));
}


void BasicRoad::disconnectAll(BasicRoad* road)
{
	while (road->m_connections.size())
	{
		dismissConnection(*road->m_connections.begin());
	}
}
