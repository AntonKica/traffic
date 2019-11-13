#ifndef RES_CREATOR
#define RES_CREATOR

#include <vector>
#include <map>
#include <iostream>

#include "Grid.h"
#include "GridTileObject.h"
#include "BasicRoad.h"
#include "BasicBuilding.h"
#include "RoadCurve.h"
// shoulld be singleton
namespace MaxAllocations
{
	constexpr size_t MAX_TILES = 100;
}

namespace
{
	using GridTileResource = GridTile::ObjectType;
}

class resourceCreator
{
public:
	//~resourceCreator() = delete;
	resourceCreator(const resourceCreator&) = delete;
	resourceCreator operator=(const resourceCreator&) = delete;
	resourceCreator(const resourceCreator&&) = delete;
	resourceCreator operator ==(const resourceCreator&&) = delete;

	~resourceCreator()
	{
		for (const auto& vector : m_gridTileObject)
		{
			for (auto resource : vector.second)
			{
				delete resource;
			}
		}
	}


	GridTileObject* createGridTileResource(GridTileResource createType)
	{
		GridTileObject* newObject = getPrototype(createType);

		m_gridTileObject[createType].push_back(newObject);
		return newObject;
	}


	GridTileObject* getPrototype(GridTileResource createType)
	{
		//deduce type
		switch (createType)
		{
		case GridTileResource::ROAD:
			return new BasicRoad;
		case GridTileResource::CURVE:
			return new RoadCurve;
		case GridTileResource::BUILDING:
			return new BasicBuilding;
		default:
			throw std::runtime_error("Wrong or undefined object creattion type!");
		}
	}
	// EXPERIMENTAL
	std::vector<Models::TexturedVertex> resourceVertices(GridTileResource resourceType)
	{
		//deduce type
		switch (resourceType)
		{
		case GridTileResource::ROAD:
			return BasicRoad().getVertices();
		case GridTileResource::CURVE:
			return RoadCurve().getVertices();
		case GridTileResource::BUILDING:
			return BasicBuilding().getVertices();
		default:
			throw std::runtime_error("Wrong or undefined object creattion type!");
		}
	}
	// EXPERIMENTAL AS WELL
	// EXPERIMENTAL
	std::vector<uint32_t> resourceIndices(GridTileResource resourceType)
	{
		//deduce type
		switch (resourceType)
		{
		case GridTileResource::ROAD:
			return BasicRoad().getIndices();
		case GridTileResource::CURVE:
			return RoadCurve().getIndices();
		case GridTileResource::BUILDING:
			return BasicBuilding().getIndices();
		default:
			throw std::runtime_error("Wrong or undefined object creattion type!");
		}
	}
	std::string resourceTexture(GridTileResource resourceType)
	{
		//deduce type
		switch (resourceType)
		{
		case GridTileResource::ROAD:
			return BasicRoad().getTexturePath();
		case GridTileResource::CURVE:
			return RoadCurve().getTexturePath();
		case GridTileResource::BUILDING:
			return BasicBuilding().getTexturePath();
		default:
			throw std::runtime_error("Wrong or undefined object creattion type!");
		}
	}
	void initResourceCrator()
	{
		for (GridTileResource type =
			static_cast<GridTileResource>(0);
			type < GridTileResource::MAX_OBJECT_TYPES; ++type)
		{
			m_gridTileObject[type];
			m_maxAllocations[type] = GridSettings::maxTiles();
		}
	}

	const std::vector<GridTileObject*>& getObjects(const GridTileResource type)
	{
		return m_gridTileObject[type];
	}

	size_t getMaxAllocations(const GridTileResource type)
	{
		return m_maxAllocations[type];
	}
	size_t getResourceCount(const GridTileResource type)
	{
		return m_gridTileObject[type].size();
	}

private:
	// experimental
	resourceCreator()
	{
		initResourceCrator();
	}
	enum idEnum { EN };
	static friend resourceCreator& CreateResourceCreator();

	std::map<GridTileResource, std::vector<GridTileObject*>> m_gridTileObject;
	std::map<GridTileResource, size_t> m_maxAllocations;
};

static resourceCreator& CreateResourceCreator()
{
	static resourceCreator resourceCreator;

	return resourceCreator;
}

#endif RES_CREATOR