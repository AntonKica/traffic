#include "GridTileObject.h"
#include <iostream>
#include "Grid.h"

GridTileObject::GridTileObject()
{
	m_size = glm::ivec2(1, 1);
	m_position = { 0,0 };
	m_rotation = 0.0;
}

GridTileObject::~GridTileObject()
{
}

void GridTileObject::placeOnGrid(std::vector<GridTile*> position)
{
	for (GridTile* occupiedTile : m_occupiedSpace)
	{
		occupiedTile->emptyTile();
	}

	m_occupiedSpace = position;
	for (GridTile* occupiedTile : m_occupiedSpace)
	{
		occupiedTile->placeObject(this);
	}
	m_position = getAveragePosition(m_occupiedSpace);

	placeOnGridAction();
}

void GridTileObject::placeOnGridAction()
{
}

void GridTileObject::setPosition(glm::dvec2 newPos)
{
	m_position = newPos;
}

glm::dvec2 GridTileObject::getPosition() const
{
	return m_position;
}

glm::dvec3 GridTileObject::getWorldPosition() const
{
	return glm::dvec3(m_position.x, 0.0, m_position.y) + getRelativePosition();
}


double GridTileObject::getRotation() const
{
	return m_rotation;
}

void GridTileObject::setRotation(double rotation)
{
	m_rotation = rotation;
}

void GridTileObject::rotate(Rotate direction)
{
	switch (direction)
	{
	case GridTileObject::Rotate::CLOCKWISE:
		m_rotation += 90;
		break;
	case GridTileObject::Rotate::COUNTERCLOCKWISE:
		m_rotation -= 90;
		break;
	}

	// out of bounds
	if (m_rotation <= -90)
		m_rotation = 270;
	else if (m_rotation >= 360)
		m_rotation = 0;
}

glm::dvec2 GridTileObject::getAveragePosition(const std::vector<GridTile*>& tiles)
{
	glm::dvec2 positionsCount{};
	// find average of positions
	for (const GridTile* tile : tiles)
	{
		positionsCount += tile->getPosition();
	}

	return positionsCount / static_cast<double>(tiles.size());
}

bool GridTileObject::compareGridTileObject(const GridTileObject& objectOne, const GridTileObject& objectTwo)
{
	glm::dvec2 averageOne = getAveragePosition(objectOne.m_occupiedSpace);
	glm::dvec2 averageTwo = getAveragePosition(objectTwo.m_occupiedSpace);

	// comapre positions
	return averageOne.y == averageTwo.y ? 
		averageOne.x < averageTwo.x :		// if on the same row, comapare column pos
		averageOne.y < averageTwo.y;		// if not on same, compare them
}

GridTileObject::AdjacencyFlags GridTileObject::getAdjacency(const GridTileObject& gridTileObjectOne, const GridTileObject& gridTileObjectTwo)
{
	auto pos1 = gridTileObjectOne.getWorldPosition();
	auto pos2 = gridTileObjectTwo.getWorldPosition();

	glm::dvec3 diff = glm::abs(pos1 - pos2);
	
	AdjacencyFlags flags{};
	if (diff.x <= 1 && diff.z <= 1)
		flags |= AdjacencyBits::Neighbour;
	else
		flags |= AdjacencyBits::Distant;

	// we suppose that pos1 != pos2
	if (diff.x == 0 && diff.z != 0)
		flags |= AdjacencyBits::LinearZ | AdjacencyBits::Linear;
	else if (diff.x != 0 && diff.z == 0)
		flags |= AdjacencyBits::LinearX | AdjacencyBits::Linear;
	else
		flags |= AdjacencyBits::Diagonal;

	return flags;
}
