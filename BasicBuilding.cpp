#include "BasicBuilding.h"

BasicBuilding::BasicBuilding()
{
}

std::vector<Models::TexturedVertex> BasicBuilding::getVertices() const
{
	static std::vector<Models::TexturedVertex> s_vertices = 
	{
		// bottom side
		{-0.5, -0.5, 0.5, 0.0, 1.0},
		{ 0.5, -0.5, 0.5, 1.0, 1.0},
		{ 0.5, -0.5,-0.5, 1.0, 0.0},
		{-0.5, -0.5,-0.5, 0.0, 0.0},
		// top side
		{-0.5, 0.5, 0.5, 0.0, 1.0},
		{ 0.5, 0.5, 0.5, 1.0, 1.0},
		{ 0.5, 0.5,-0.5, 1.0, 0.0},
		{-0.5, 0.5,-0.5, 0.0, 0.0}
	};

	return s_vertices;
}

std::vector<uint16_t> BasicBuilding::getIndices() const
{
	static std::vector<uint16_t> s_indices =
	{
		// bottom plane
		2,1,0,
		0,3,2,
		// top plane
		4,5,6,
		6,7,4,
		// left plane
		0,4,7,
		7,3,0,
		// right plane
		1,2,6,
		6,5,1,
		// front plane
		0,1,5,
		5,4,0,
		// back plane
		2,3,7,
		7,6,2
	};

	return s_indices;
}

GridTile::ObjectType BasicBuilding::getObjectType() const
{
	return GridTile::ObjectType::BUILDING;
}

std::string BasicBuilding::getTexturePath() const
{
	static std::string s_texturePath = "resources/materials/building.png";
	return s_texturePath;
}


glm::dvec3 BasicBuilding::getRelativePosition() const
{
	return glm::dvec3(0, 0.5, 0);
}
