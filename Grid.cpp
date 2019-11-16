#include "Grid.h"

#include <vector>
#include <algorithm>
#include <optional>
#include <set>
#include <chrono>

#include <glm/gtc/constants.hpp>

#include "GlobalObjects.h"


static std::optional<glm::vec3> intersectPoint(glm::dvec3 rayDir, glm::dvec3 rayPos,
	glm::dvec3 planeNormal, glm::dvec3 planePos)
{
	std::optional<glm::dvec3> returnVal;
	double d = dot(rayDir, planeNormal);

	if (abs(d) > 0)
	{
		double distance = glm::length(planePos - rayPos);
		double t = -(glm::dot(rayPos, planeNormal)) / d;

		returnVal = (planePos + rayPos) + rayDir * t;
	}

	return returnVal;
}

Grid::Grid()
{
}

void Grid::initTiles()
{
	double tilePerUnit = getTilesPerUnit();

	// endign tike
	m_xTileCount= getTilesPerLength (m_width) + 1;
	m_zTileCount = getTilesPerLength (m_height) + 1;

	double halfXSize = m_xTileCount / 2;
	double halfZSize = m_xTileCount / 2;

	m_tiles.resize(m_xTileCount);
	for (int x = 0; x < m_tiles.size(); ++x)
	{
		m_tiles[x].resize(m_zTileCount);
		for (int z = 0; z < m_tiles[x].size(); ++z)
		{
			glm::dvec2 pos (x - halfXSize, z - halfZSize);
			pos *= m_tileSize;

			GridTile newTile(pos);

			m_tiles[x][z] = newTile;
		}
	}

}

void Grid::initGrid(const std::map<GridTile::ObjectType, std::vector<std::pair<glm::dvec2, double>>>& initList)
{
	// + 1 
	m_width = GridSettings::defaultGridSize;
	m_height = GridSettings::defaultGridSize;
	m_tileSize = GridSettings::defaultTileSize;

	initTiles();

	// place on grid
	for (const auto&[objType, objects] : initList)
	{
		for (const auto& [pos, rot] : objects)
		{
			placeObject(objType, pos, rot);
		}
	}

	initGraphicsComponent();
	/**	// dont mind
	BasicRoad* currentRoad = nullptr;
	auto lambda = [](const glm::dvec3& p1, const glm::dvec3& p2) { return p1.z > p2.z || p1.x > p2.x; };
	for (auto& xTiles : m_tiles)
	{
		for (auto& tile : xTiles)
		{
			if (tile.getStatus() == GridTile::ObjectType::ROAD || tile.getStatus() == GridTile::ObjectType::CURVE)
			{
				BasicRoad* compRoad = dynamic_cast<BasicRoad*>(tile.getPlacedObject());
				if (!currentRoad)
				{
					currentRoad = compRoad;
				}
				else
				{
					auto pos1 = currentRoad->getWorldPosition();
					auto pos2 = compRoad->getWorldPosition();

					if (lambda(pos1, pos2))
					{
						currentRoad = compRoad;
					}
				}
			}
		}
	}
	
	// genPath
	BasicRoad* beginRoad = currentRoad;
	std::vector<glm::dvec2> generatedPath;
	while (currentRoad != nullptr && generatedPath.size() <= 100)
	{
		auto path = currentRoad->getPath();

		generatedPath.reserve(path.points.size());
		std::copy(path.points.begin(), path.points.end(), std::back_inserter(generatedPath));

		int counter = 0;
		for (auto ep = BasicRoad::EntryPoint::FRONT; ep != BasicRoad::EntryPoint::MAX_ENTRY_POINTS && counter < 4; ++ep)
		{
			auto nextRoad = currentRoad->getConnectedRoad(ep);

			if (nextRoad != nullptr)
			{
				currentRoad = nextRoad;
				break;
			}
			++counter;
		}
		// infinite loop
		if (currentRoad == beginRoad)
			break;
	}

	generatedPath.erase(std::unique(generatedPath.begin(), generatedPath.end()), generatedPath.end());
	
	auto lambda2 = [](const glm::dvec2& p1, const glm::dvec2& p2) -> bool
	{ 
		return p1.x > p2.x && p1.y > p2.y;
	};
	std::sort(generatedPath.begin(), generatedPath.end(), lambda2);

	std::cout << "Path is ";
	for (const auto& point : generatedPath)
	{
			std::cout << '(' << point.x << ", " << point.y << ')';
	}
	std::cout << '\n';*/
}

std::vector<GridTile*> Grid::getSelectedTiles() const
{
	return m_selectedTiles;
}

size_t Grid::getSelectedTilesCount() const
{
	return m_selectedTiles.size();
}

int Grid::getTilesCount()
{
	return m_tiles.size() * m_tiles[0].size();
}

std::pair<uint32_t, uint32_t> Grid::getGridLinesCount() const
{
	return std::pair<uint32_t, uint32_t>(m_xTileCount + 1, m_zTileCount + 1);
}


double Grid::getGridLineLength() const
{
	return m_width + m_tileSize;
}

GridTile* Grid::getTile(double x, double z)
{
	return getTile(glm::dvec2(x, z));
}

GridTile* Grid::getTile(glm::dvec2 pos)
{
	glm::dvec2 p = transformToGridSpace(pos);
	p = transformToTileSpace(p);

	return &m_tiles[p.x][p.y];
}

std::vector<GridTile*> Grid::getSurroundingTiles(double x, double z)
{
	return getSurroundingTiles(glm::dvec2(x, z));
}

std::vector<GridTile*> Grid::getSurroundingTiles(glm::dvec2 pos)
{
	// 8 blocks
	std::vector<GridTile*> surrounding;
	// simpel solution to off bound
	for (int x = 0; x < 3; ++x)
	{
		for (int z = 0; z < 3; ++z)
		{
			if (x == 1 && z ==1)
			{
				continue;
			}

			glm::dvec2 p = { pos.x + x - 1, pos.y + z - 1 };
			
			auto tiles = getGridSpace(p);
			surrounding.insert(surrounding.end(), tiles.begin(), tiles.end());
		}
	}

	return surrounding;
}

int Grid::getTilesPerUnit() const
{
	return 1.0 / m_tileSize;
}

inline int Grid::getTilesPerLength(int size) const
{
	return getTilesPerUnit() * size;
}

std::vector<GridTile*> Grid::getGridSpace(glm::dvec2 position)
{
	// we supose GTPU is > 1 and is integral
	int xSpace = getTilesPerLength(1);
	int zSpace = getTilesPerLength(1);

	auto roundedPos = roundToTileUnits(position);
	std::vector<GridTile*> gridSpace;

	uint8_t index = 0;
	for (int x = 0; x < xSpace; ++x)
	{
		for (int z = 0; z < zSpace; ++z)
		{
			double xPos = roundedPos.x + m_tileSize *(x - xSpace / 2.0);
			double zPos = roundedPos.y + m_tileSize *(z - zSpace / 2.0);

			glm::dvec2 tilePos(xPos, zPos);
			if (isOnGrid(tilePos))
			{
				gridSpace.push_back(getTile(tilePos));
			}
			++index;
		}
	}

	return gridSpace;
}

GridTile* Grid::getSelectedTile() const
{
	return m_selectedTile;;
}

//For bound checking
bool Grid::isOnGrid(double x, double z) const
{
	return isOnGrid(glm::dvec2(x, z));
}
//For bound checking
bool Grid::isOnGrid(glm::dvec2 pos) const
{
	pos = transformToTileSpace(transformToGridSpace(pos));

	return pos.x >= 0 && pos.x < m_xTileCount && pos.y >= 0 && pos.y < m_zTileCount;
}

glm::dvec2 Grid::transformToGridSpace(const glm::dvec2& position) const
{
	glm::dvec2 newPos = position;
	newPos.x += m_width / 2;
	newPos.y += m_height / 2;

	return newPos;
}

inline glm::ivec2 Grid::transformToTileSpace(const glm::dvec2& position) const
{
	glm::dvec2 newPos = position;
	newPos /= m_tileSize;

	return newPos;
}

// this function rounds units in positive direction
inline glm::dvec2 Grid::roundToTileUnits(const glm::dvec2& position) const
{
	// calculated according to number of significant digits in tile count
	// e.g if 8 tiles per unit, then 1 / 8 = 0.125 hence 3 significant digits, so log2(8) = 2
	static int dstFromDPoint = 10 * (std::log10(GridSettings::tilesPerUnit) / std::log10(2));
	static int tileMultyplies = m_tileSize * dstFromDPoint;

	glm::ivec2 superPos(position * double(dstFromDPoint));
	glm::ivec2 remainder(superPos % dstFromDPoint);

	auto determinator = [&](int pos)
	{
		bool negative = pos < 0;
		// turn to range 0 - tileMultyplies
		int substract = pos / tileMultyplies;
		int result = std::abs(pos - tileMultyplies * substract);

		// round top positive
		return negative ? result : tileMultyplies - result;
	};

	superPos.x += determinator(remainder.x);
	superPos.y += determinator(remainder.y);

	return glm::dvec2(superPos) / double(dstFromDPoint);
}

GO::TypedVertices Grid::createGridLines() const
{
	GO::TypedVertices vVerts;
	vVerts.first = GO::VertexType::DEFAULT;

	auto createAxialySymmetricalVertices = [](const glm::vec3& vertex, double distance, bool horizontal)
		->std::pair< glm::vec3, glm::vec3>
	{
		glm::vec3 first, second;
		if (horizontal)
		{
			first = glm::vec3(vertex.x, vertex.y, distance / 2.0);
			second = glm::vec3(vertex.x, vertex.y, -distance / 2.0);
		}
		else
		{
			first = glm::vec3(distance / 2.0, vertex.y, vertex.z);
			second = glm::vec3(-distance / 2.0, vertex.y, vertex.z);
		}
		return std::make_pair(first, second);
	};

	const auto [xLines, zLines] = getGridLinesCount();
	double burryY = -0.01;
	for (int nLine = 0; nLine < xLines; ++nLine)
	{
		double halfWidth = m_width / 2.0;
		double offset = -m_tileSize / 2.0;
		double posX = -halfWidth + (nLine * m_tileSize) + offset;

		glm::vec3 pos = glm::vec3(posX, burryY, 0.0);

		double zSpan = m_height / 2.0;
		const auto [top, bottom] = createAxialySymmetricalVertices(pos, zSpan, true);

		GO::VariantVertex verticalTop;
		verticalTop.vertex.position = top;
		GO::VariantVertex verticalBottom;
		verticalBottom.vertex.position = bottom;

		vVerts.second.insert(vVerts.second.begin(), { verticalTop, verticalBottom });
	}

	for (int nLine = 0; nLine < zLines; ++nLine)
	{
		double halfHeight = m_height / 2.0;
		double offset = -m_tileSize / 2.0;
		double posZ = -halfHeight + (nLine * m_tileSize) + offset;

		glm::vec3 pos = glm::vec3(0.0, burryY, posZ);

		double xSpan = m_width / 2.0;
		const auto [left, right] = createAxialySymmetricalVertices(pos, xSpan, false);

		GO::VariantVertex verticalLeft;
		verticalLeft.vertex.position = left;
		GO::VariantVertex verticalRight;
		verticalRight.vertex.position = right;

		vVerts.second.insert(vVerts.second.begin(), { verticalLeft, verticalRight });
	}
	return vVerts;
}

void Grid::initGraphicsComponent()
{
	Info::DrawInfo draw;
	draw.lineWidth = 1.0;
	draw.polygon = VK_POLYGON_MODE_FILL;
	draw.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	Info::ModelInfo model;
	auto verts = createGridLines();
	model.vertices = &verts;

	Info::GraphicsComponentCreateInfo info;
	info.drawInfo = &draw;
	info.modelInfo = &model;

	gridGraphics = App::Scene.vulkanBase->createGrahicsComponent(info);
}

void Grid::clickEvent()
{
	if (m_selectedTiles.empty())
		return;

	std::vector<GridTile*> placement;

	// check for emptynes
	bool canPlace = true;
	for (GridTile* selectedTile : m_selectedTiles)
	{
		if (selectedTile)
		{
			if (selectedTile->getStatus() != GridTile::ObjectType::EMPTY)
			{
				canPlace = false;
				break;
			}
			placement.push_back(selectedTile);
		}
		else
		{
			break;
		}
	}

	if (canPlace)
	{
		auto obj = m_creator.getObject();
		placeObject(obj->getObjectType(), obj->getPosition(), obj->getRotation());
	}
}

void Grid::updateSelectedTile()
{
	// clean selection
	m_selectedTiles.clear();
	if (!m_enableMouse)
		return;

	glm::dvec3 normal = glm::cross(glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0));
	std::optional <glm::dvec3> intersectResult = intersectPoint(App::Scene.m_camera.getMouseRay(), App::Scene.m_camera.getPosition(), normal, m_position);

	if (!intersectResult.has_value())
		return;

	glm::dvec2 pos = { intersectResult.value().x, intersectResult.value().z };
	if(isOnGrid(pos))
		m_selectedTile = getTile(pos);

	auto selection = getGridSpace(pos);
	// filter
	auto badSection = std::remove_if(selection.begin(), selection.end(), [](const auto& tile) { return tile == nullptr; });
	m_selectedTiles.insert(m_selectedTiles.begin(), selection.begin(), badSection);
}

void Grid::setMouseRay(glm::vec3 ray)
{
	m_mouseRay = ray;
}

void Grid::setMouseEnable(bool enable)
{
	m_enableMouse = enable;
}

void Grid::placeObject(GridTile::ObjectType objectType, glm::dvec2 position, double rotation)
{
	GridTileObject* newObject = App::Scene.m_resourceCreator.createGridTileResource(objectType);
	newObject->setRotation(rotation);

	auto placement = getGridSpace(position);
	newObject->placeOnGrid(placement);
}

void Grid::placeObject(const GridTileObject* const object)
{
	GridTileObject* newObject = App::Scene.m_resourceCreator.createGridTileResource(object->getObjectType());
	*newObject = *object;

	auto placement = getGridSpace({ newObject->getPosition()});
	newObject->placeOnGrid(placement);
}

void Grid::update(double deltaTime)
{
	m_creator.update();
}
