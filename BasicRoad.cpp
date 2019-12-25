#include "BasicRoad.h"


BasicRoad::BasicRoad()
{
}

BasicRoad::~BasicRoad()
{
	disconnectAll();
}

BasicRoad::BasicRoad(const BasicRoad& copy)
	: SimulationAreaObject(copy)
{
	copy.copyConnections(this);
}

BasicRoad::BasicRoad(BasicRoad&& move)
	:SimulationAreaObject(move)
{
	move.transferConnections(this);
}

BasicRoad& BasicRoad::operator=(const BasicRoad& copy)
{
	copy.copyConnections(this);

	// TODO: insert return statement here
	return *this;
}

BasicRoad& BasicRoad::operator=(BasicRoad&& move)
{
	move.transferConnections(this);
	// TODO: insert return statement here
	return *this;
}


/*BasicRoad::Connection& BasicRoad::findConnection(BasicRoad* connectedRoad)
{
	return *std::find(std::begin(m_connections), std::end(m_connections), connectedRoad);
}*/

BasicRoad::Connection& BasicRoad::findConnection(Point connectedPoint)
{
	return *std::find(std::begin(m_connections), std::end(m_connections), connectedPoint);
}

void BasicRoad::connect(BasicRoad* connectionRoad, Point connectionPoint)
{
	Connection connection;
	connection.point = connectionPoint;
	connection.connected = connectionRoad;
	m_connections.push_back(connection);

	connection.connected = this;
	connectionRoad->m_connections.push_back(connection);
}


void BasicRoad::addConnection(Connection connection)
{
	connect(connection.connected, connection.point);
}

void BasicRoad::copyConnections(BasicRoad* destinationRoad) const
{
	for (auto& connection : m_connections)
		destinationRoad->addConnection(connection);
}

void BasicRoad::transferConnections(BasicRoad* destinationRoad)
{
	copyConnections(destinationRoad);

	disconnectAll();
}

void BasicRoad::dismissConnection(Connection& connection)
{
	disconnect(connection.point);
}


void BasicRoad::disconnect(Point point)
{
	auto& otherConnections = findConnection(point).connected->m_connections;

	// no control
	m_connections.erase(std::find(std::begin(m_connections), std::end(m_connections), point));
	otherConnections.erase(std::find(std::begin(otherConnections), std::end(otherConnections), point));
}


void BasicRoad::disconnectAll()
{
	while (m_connections.size())
	{
		dismissConnection(*m_connections.begin());
	}
}
