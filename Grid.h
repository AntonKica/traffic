#ifndef GRID_h
#define GRID_h

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>
#include <map>

#include "GridTile.h"
#include "GridTileObjectCreator.h"
#include "GraphicsComponent.h"

class GridTile;
class GridTileObject;

constexpr bool multipleOfTwo(int num)
{
	return (num != 0) && (num & (num - 1)) == 0;
}

namespace GridSettings
{
	constexpr int tilesPerUnit = 8;
	static_assert(tilesPerUnit > 1 && tilesPerUnit < 32 && multipleOfTwo(tilesPerUnit)
		&& "Tile per one unit > 1 and < 32 and is multiple of 2");

	constexpr double defaultTileSize = 1.0 / tilesPerUnit;
	constexpr int defaultGridSize = 100;
	static_assert(defaultGridSize <= 2000 &&
		"Wth, nejako vela");
	constexpr size_t maxTiles() { return defaultGridSize * defaultGridSize; }

	constexpr int maxSelectedTiles = (1/ defaultTileSize) * (1 / defaultTileSize);
}

//tiles is goint to be
class Grid
{
public:
	Grid();

	void initGrid(const std::map<GridTile::ObjectType, std::vector<std::pair<glm::dvec2, double>>>& initList = {});
	void initTiles();

	// getters
	std::vector<GridTile*> getSelectedTiles() const;
	size_t getSelectedTilesCount()const;
	GridTile* getTile(double x, double z);
	GridTile* getTile(glm::dvec2 pos);
	int getTilesCount();

	std::pair<uint32_t, uint32_t> getGridLinesCount() const;
	double getGridLineLength() const;

	std::vector<GridTile*> getSurroundingTiles(double x, double z);
	std::vector<GridTile*> getSurroundingTiles(glm::dvec2 pos);

	inline int getTilesPerUnit() const;
	inline int getTilesPerLength(int size) const;
	std::vector<GridTile*> getGridSpace(glm::dvec2 position);

	GridTile* getSelectedTile() const;

	// for bound checking
	bool isOnGrid(double x, double z) const;
	bool isOnGrid(glm::dvec2 pos) const;


	void clickEvent();
	void updateSelectedTile();

	void setMouseRay(glm::vec3 ray);
	void setMouseEnable(bool enable);

	void placeObject(GridTile::ObjectType objectType, glm::dvec2 position, double rotation);
	void placeObject(const GridTileObject* const object);

	void update(double deltaTime);

	// mouse
	glm::vec3 m_position;
	glm::vec3 m_mouseRay;
	bool m_enableMouse;


	// settings
	int m_width, m_height;
	double m_tileSize;

	uint32_t m_xTileCount, m_zTileCount;
	std::vector<std::vector<GridTile>> m_tiles;
	std::vector<GridTile*> m_selectedTiles;
	GridTile* m_selectedTile;

	GridTileObjectCreator m_creator;
private:
	inline glm::dvec2 transformToGridSpace(const glm::dvec2& position) const;
	inline glm::ivec2 transformToTileSpace(const glm::dvec2& position) const;
	// this function rounds units in positive direction
	inline glm::dvec2 roundToTileUnits(const glm::dvec2& position) const;
	GO::TypedVertices createGridLines() const;
	void initGraphicsComponent();

	const GraphicsComponent* gridGraphics;
};
#endif // !GRID_h
