#pragma once
#include <map>
#include "GridTile.h"
#include "Utilities.h"
#include "GraphicsComponent.h"

class GridTileObjectCreator : public Utility::NonCopy
{
private:
	// private
	void initResources();
	void releaseResources();
	void updateObjectPos();
	void initGraphics();
	void updateGraphics();
public:
	// no init function
	GridTileObjectCreator();
	~GridTileObjectCreator();
	// Rework this method!
	void processKeyInput(int key, int status);
	// raw
	void setCreateObject(int type);
	void setCreateObject(GridTile::ObjectType type);
	void update();

	// getters
	const GridTileObject* const getObject() const;
	GridTile::ObjectType getCurrentType() const;
private:
	// not public
	std::map<GridTile::ObjectType, GridTileObject*> m_prototypes;
	GridTile::ObjectType m_currentType;
	GridTileObject* m_currentObject;
	// trying
	const GraphicsComponent* graphics;
};

