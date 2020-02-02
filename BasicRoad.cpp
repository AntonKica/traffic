#include "BasicRoad.h"


BasicRoad::BasicRoad()
{
}

BasicRoad::~BasicRoad()
{
	disconnectAll();
}

BasicRoad::BasicRoad(const BasicRoad& copy)
	: SimulationObject(copy)
{
	copy.copyConnections(this);
}


BasicRoad::BasicRoad(BasicRoad&& move)
	:SimulationObject(std::move(move))
{
	move.transferConnections(this);
}


BasicRoad& BasicRoad::operator=(const BasicRoad& copy)
{
	SimulationObject::operator=(copy);

	disconnectAll();
	copy.copyConnections(this);

	// TODO: insert return statement here
	return *this;
}



BasicRoad& BasicRoad::operator=(BasicRoad&& move)
{
	SimulationObject::operator=(std::move(move));

	disconnectAll();

	move.transferConnections(this);
	// TODO: insert return statement here
	return *this;
}

uint32_t BasicRoad::getConnectedCount() const
{
	return m_connections.size();
}

/*BasicRoad::Connection& BasicRoad::findConnection(BasicRoad* connectedRoad)
{
	return *std::find(std::begin(m_connections), std::end(m_connections), connectedRoad);
}*/



const std::vector<BasicRoad::Connection>& BasicRoad::getConnections() const
{
	return m_connections;
}

bool BasicRoad::canConnect(Point point) const
{
	for (const auto& connection : m_connections)
	{
		if (approxSamePoints(connection.point, point))
			return false;
	}

	return true;
}

const BasicRoad::Connection& BasicRoad::getConnection(Connection connection) const
{
	return *std::find(std::begin(m_connections), std::end(m_connections), connection);
}

const BasicRoad::Connection& BasicRoad::getConnection(BasicRoad* road, Point point) const
{
	return getConnection({ road, point });
}

/*
* Carefully with this
*/
const BasicRoad::Connection& BasicRoad::getConnection(Point point) const
{
	return *std::find(std::begin(m_connections), std::end(m_connections), point);
}

const BasicRoad::Connection* BasicRoad::findConnection(Point point) const
{
	auto findIter = std::find(std::begin(m_connections), std::end(m_connections), point);

	return findIter != std::end(m_connections) ? &*findIter : nullptr;
}

BasicRoad* BasicRoad::findConnectedRoad(Point point) const
{
	auto findIter = std::find(std::begin(m_connections), std::end(m_connections), point);

	return findIter != std::end(m_connections) ? findIter->connected : nullptr;
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

void BasicRoad::dismissConnection(Connection connection)
{
	auto& thisConnection = getConnection(connection);
	auto& otherConnection = thisConnection.connected->getConnection(this, connection.point);

	//erase from ours
	m_connections.erase(std::find(std::begin(m_connections), std::end(m_connections), thisConnection));

	// erase from other too
	auto& otherConnections = connection.connected->m_connections;
	otherConnections.erase(std::find(std::begin(otherConnections), std::end(otherConnections), otherConnection));
}

void BasicRoad::disconnectAll()
{
	while (m_connections.size())
	{
		dismissConnection(*m_connections.begin());
	}
}

Path BasicRoad::getClosestPath(Point pt) const
{
	auto closestTraiPoint = [](const Trail& trail, Point pt)
	{
		// find edges
		const auto [p1, p2] = findTwoClosestPoints(trail, pt);
		return getClosestPointToLine(p1, p2, pt);
	};

	const Path* pPath = nullptr;
	Point minPt = {};

	for (auto& [side, paths] : m_paths)
	{
		for (auto& path : paths)
		{
			if (!pPath)
			{
				pPath = &path;
				minPt = closestTraiPoint(pPath->points, pt);
			}
			else
			{
				auto curPt = closestTraiPoint(path.points, pt);

				if (glm::length(pt - curPt) < glm::length(pt - minPt))
					pPath = &path;
			}
		}
	}

	return *pPath;
}

std::vector<Path> BasicRoad::getSubsequentPathsFromConnectingPath(const Path& connectingPath) const
{
	std::vector<Path> subsequentPaths;
	for (const auto& [side, paths] : m_paths)
	{
		for (const auto& path : paths)
		{
			// end from cp myst be approximately in begin in some of our paths
			if (Path::pathLeadsToPath(connectingPath, path))
				subsequentPaths.emplace_back(path);
		}
	}

	return subsequentPaths;
}

std::vector<Path> BasicRoad::getAllPathsConnectingTo(const BasicRoad* const connectsToRoad) const
{
	std::vector<Path> connectingPaths;
	for (const auto& [side, paths] : m_paths)
	{
		for (const auto& path : paths)
		{
			// end from cp myst be approximately in begin in some of our paths
			if (path.connectsTo == connectsToRoad)
				connectingPaths.emplace_back(path);
		}
	}

	return connectingPaths;
}

std::vector<Path> BasicRoad::getAllPathsConnectingTwoRoads(
	const BasicRoad* const connectsFromRoad,
	const BasicRoad* const connectsToRoad) const
{
	std::vector<Path> subsequentPaths;
	for (const auto& [side, paths] : m_paths)
	{
		for (const auto& path : paths)
		{
			// end from cp myst be approximately in begin in some of our paths
			if (path.connectsFrom == connectsFromRoad && path.connectsTo == connectsToRoad)
				subsequentPaths.emplace_back(path);
		}
	}

	return subsequentPaths;
}

bool Path::pathLeadsToPath(const Path& path, const Path& leadsToPath)
{
	return approxSamePoints(path.points.back(), leadsToPath.points.front());
}

bool operator==(const Path& lhs, const Path& rhs)
{
	bool pointsEqual = lhs.points.size() == rhs.points.size() &&
		std::memcmp(lhs.points.data(), rhs.points.data(), sizeof(lhs.points[0]) * lhs.points.size() == 0);

	return 	pointsEqual &&
		lhs.connectsFrom == rhs.connectsFrom &&
		lhs.connectsTo == rhs.connectsTo &&
		lhs.side == rhs.side;
}
