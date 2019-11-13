#pragma once
#include "GridTileObject.h"
class BasicBuilding :
	public GridTileObject
{
public:
	BasicBuilding();

	std::vector<Models::TexturedVertex> getVertices() const override;
	std::vector<uint16_t> getIndices() const override;
	GridTile::ObjectType getObjectType() const override;
	virtual std::string getTexturePath() const;

	glm::dvec3 getRelativePosition() const override;
private:

};

