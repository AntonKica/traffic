#ifndef BASIC_ROAD_H
#define BASIC_ROAD_H

#include "GridTileObject.h"
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

class BasicRoad :
	public GridTileObject
{
public:
	// clock-wie
	enum class EntryPoint
	{
		FRONT,
		RIGHT,
		BACK,
		LEFT,
		MAX_ENTRY_POINTS
	};
	friend EntryPoint& operator++(EntryPoint& ep)
	{
		ep = static_cast<EntryPoint>(static_cast<int>(ep) + 1);

		// reset rotation
		if (ep == EntryPoint::MAX_ENTRY_POINTS)
			ep = static_cast<EntryPoint>(0);

		return ep;
	}
	friend EntryPoint operator+(const EntryPoint& ep, int num)
	{
		EntryPoint newEp = ep;
		for (int i = 0; i < num; ++i)
			++newEp;

		return newEp;
	}
	
	static bool entryPointOpposite(EntryPoint e1, EntryPoint e2);
	static bool entryPointOfXAxis(const EntryPoint& e);
	static bool entryPointOfZAxis(const EntryPoint& e);
	static void connectRoads(BasicRoad& lhs, BasicRoad& rhs);
	static bool canConnect(const BasicRoad& lhs, const BasicRoad& rhs);
	static bool alreadyConnected(const BasicRoad& lhs, const BasicRoad& rhs);

	BasicRoad();

	virtual std::vector<EntryPoint> getEntryPoints() const;
	BasicRoad* getConnectedRoad(EntryPoint entry) const;

	const std::vector<Models::TexturedVertex>& getVertices() const override;
	const std::vector<uint32_t>& getIndices() const override;
	GridTile::ObjectType getObjectType() const override;
	std::string getTexturePath() const override;
	glm::dvec3 getRelativePosition() const override;
	void placeOnGridAction() override;

	virtual std::vector<Lane> generateLanes();
	virtual Path getPath(bool rightLane = true);


protected:
	//std::vector<EntryPoint> entryPoints;
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
	BasicRoad::EntryPoint entryPoint;
};
#endif // !BASIC_ROAD_H
