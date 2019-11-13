#include "GridTile.h"
#include "GridTileObject.h"

GridTile::GridTile()
{}

GridTile::GridTile(glm::dvec2 pos)
	:m_position(pos)
{
	m_status = GridTile::ObjectType::EMPTY;
	m_placedObject = nullptr;
}

GridTile::GridTile(double x, double z)
	: GridTile(glm::dvec2(x, z))
{}

glm::dvec2 GridTile::getPosition() const
{
	return m_position;
}

GridTile::ObjectType GridTile::getStatus() const
{
	return m_status;
}

GridTileObject* GridTile::getPlacedObject()
{
	return m_placedObject;
}

void GridTile::placeObject(GridTileObject* object)
{
	m_placedObject = object;
	m_status = m_placedObject->getObjectType();
}

void GridTile::emptyTile()
{
	m_placedObject = nullptr;
	m_status = GridTile::ObjectType::EMPTY;
}