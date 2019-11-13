#include "GridTileObjectCreator.h"
#include "GlobalObjects.h"

#include <GLFW/glfw3.h>

void GridTileObjectCreator::initResources()
{
	for (GridTile::ObjectType resType = static_cast<GridTile::ObjectType>(0);
		resType < GridTile::ObjectType::MAX_OBJECT_TYPES;
		++resType)
	{
		m_prototypes[resType] = App::Scene.m_resourceCreator.getPrototype(resType);
	}
}

void GridTileObjectCreator::releaseResources()
{
	for (auto& [key, obj] : m_prototypes)
	{
		delete obj;
	}
}

void GridTileObjectCreator::updateObjectPos()
{
	// ugh
	auto pos = GridTileObject::getAveragePosition(App::Scene.m_grid.getSelectedTiles());

	m_currentObject->setPosition(pos);
}

GridTileObjectCreator::GridTileObjectCreator()
{
	initResources();

	setCreateObject(0);
}

GridTileObjectCreator::~GridTileObjectCreator()
{
	releaseResources();
}

void GridTileObjectCreator::processKeyInput(int key, int status)
{
	if (status != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_Q)
	{
		m_currentObject->rotate(GridTileObject::Rotate::COUNTERCLOCKWISE);
	}
	else if(key == GLFW_KEY_E)
	{
		m_currentObject->rotate(GridTileObject::Rotate::CLOCKWISE);
	}
}

void GridTileObjectCreator::setCreateObject(int type)
{
	setCreateObject(static_cast<GridTile::ObjectType>(type));
}

void GridTileObjectCreator::setCreateObject(GridTile::ObjectType type)
{
	if (m_currentType == type && m_currentObject != nullptr)
		return;

	m_currentType = type;
	m_currentObject = m_prototypes[m_currentType];
	//m_currentObject = GridTileObject();

	updateObjectPos();
}

void GridTileObjectCreator::update()
{
	updateObjectPos();
}

const GridTileObject* const GridTileObjectCreator::getObject() const
{
	return m_currentObject;
}

GridTile::ObjectType GridTileObjectCreator::getCurrentType() const
{
	return m_currentType;
}

