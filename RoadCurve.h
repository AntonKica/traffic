#pragma once
#include "BasicRoad.h"
class RoadCurve
	: public BasicRoad
{
public:
	std::vector<EntryPoint> getEntryPoints() const override;

	GridTile::ObjectType getObjectType() const override;
	std::string getTexturePath() const override;

	virtual std::vector<Lane> generateLanes();
	virtual Path getPath(bool rightLane = true) override;
};

