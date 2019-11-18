#ifndef BASIC_ROAD_H
#define BASIC_ROAD_H

#include "SimulationAreaObjectStatic.h"

#include <glm/glm.hpp>
#include "GraphicsComponent.h"

struct Path
{
	std::vector<glm::dvec2> points;

	void addPoint(glm::dvec2 point)
	{
		points.push_back(point);
	}

	void addPoints(std::vector<glm::dvec2> newPoints)
	{
		points.insert(points.end(), newPoints.begin(), newPoints.end());
	}

	std::vector<glm::dvec2> getPath()
	{
		return points;
	}
};

struct Connection;
using Lane = Path;

namespace EP
{
	enum class EntryPoint
	{
		FRONT,
		RIGHT,
		BACK,
		LEFT,
		MAX_ENTRY_POINTS
	};
	static EntryPoint& operator++(EntryPoint& ep)
	{
		ep = static_cast<EntryPoint>(static_cast<int>(ep) + 1);

		// reset rotation
		if (ep == EntryPoint::MAX_ENTRY_POINTS)
			ep = static_cast<EntryPoint>(0);

		return ep;
	}
	static EntryPoint operator+(const EntryPoint& ep, int num)
	{
		EntryPoint newEp = ep;
		for (int i = 0; i < num; ++i)
			++newEp;

		return newEp;
	}
	static bool entryPointOpposite(const EntryPoint& e1, const EntryPoint& e2);
	static bool entryPointOfXAxis(const EntryPoint& e);
	static bool entryPointOfZAxis(const EntryPoint& e);
}

class BasicRoad :
	public SimulationAreaObjectStatic
{
public:
	static void connectRoads(BasicRoad& lhs, BasicRoad& rhs);
	static bool canConnect(const BasicRoad& lhs, const BasicRoad& rhs);
	static bool alreadyConnected(const BasicRoad& lhs, const BasicRoad& rhs);

	BasicRoad();

	virtual std::vector<EP::EntryPoint> getEntryPoints() const;
	BasicRoad* getConnectedRoad(EP::EntryPoint entry) const;


	virtual std::vector<Lane> generateLanes();
	virtual Path getPath(bool rightLane = true);


protected:
	//std::vector<EntryPoint> entryPoints;
	std::string getModelPath() const override;

	friend Connection;
	std::vector<Connection> m_connections;

	// consider making it static
	int numberOfEntryPoints;
	// consider making it static
	int m_laneCount;
	//void createPath();
};

struct Connection
{
	BasicRoad* connected;
	EP::EntryPoint entryPoint;
};
#endif // !BASIC_ROAD_H
