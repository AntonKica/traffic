#ifndef GRID_TILE_OBJECT_H
#define GRID_TILE_OBJECT_H
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "GridTile.h"
#include "Models.h"

class GridTile;

class GridTileObject
{
public:
	enum class Rotate
	{
		CLOCKWISE,
		COUNTERCLOCKWISE
	};
	enum AdjacencyBits
	{
		Neighbour	= 1 << 0,
		Distant		= 1 << 1,
		Diagonal	= 1 << 2,
		Linear		= 1 << 3,
		LinearX		= 1 << 4,
		LinearZ		= 1 << 5
	};
	typedef uint32_t AdjacencyFlags;


	static glm::dvec2 getAveragePosition(const std::vector<GridTile*>& tiles);
	static bool compareGridTileObject(const GridTileObject& gridTileObjectOne, const GridTileObject& gridTileObjectTwo);
	static AdjacencyFlags getAdjacency(const GridTileObject& gridTileObjectOne, const GridTileObject& gridTileObjectTwo);

	// should resed GridTile as well!
	GridTileObject();
	virtual ~GridTileObject();

	// EXPERIMENTAL
	virtual std::vector<Models::TexturedVertex> getVertices() const = 0;
	virtual std::vector<uint16_t> getIndices() const = 0;
	virtual std::string getTexturePath() const = 0;
	virtual GridTile::ObjectType getObjectType() const = 0;

	double getRotation() const;
	void setRotation(double rotation);
	virtual void rotate(Rotate direction);

	// weird
	void setPosition(glm::dvec2 newPos);
	glm::dvec2 getPosition() const;
	glm::dvec3 getWorldPosition() const;

	void placeOnGrid(std::vector<GridTile*> position);
	virtual void placeOnGridAction();

protected:
	virtual glm::dvec3 getRelativePosition() const = 0;

	std::vector<GridTile*> m_occupiedSpace;
	// standard should be 1
	glm::ivec2 m_size;
	glm::dvec2 m_position;
	double m_rotation;
};

#endif // !GRID_TILE_OBJECT_H
