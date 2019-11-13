#pragma once
#include <glm/glm.hpp>

class GridTileObject;

class GridTile
{
public:
	enum class ObjectType
	{
		ROAD,
		CURVE,
		BUILDING,
		MAX_OBJECT_TYPES,
		EMPTY
	};

	friend ObjectType& operator++(ObjectType& type)
	{
		return type = static_cast<ObjectType>(static_cast<int>(type) + 1);
	}

	GridTile();
	GridTile(glm::dvec2 pos);
	GridTile(double x, double z);

	glm::dvec2 getPosition() const;

	ObjectType getStatus() const;
	GridTileObject* getPlacedObject();

	void placeObject(GridTileObject* object);

	void emptyTile();

private:
	glm::dvec2 m_position;
	GridTileObject* m_placedObject;
	ObjectType m_status;
};
