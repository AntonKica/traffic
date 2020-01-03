#pragma once
#include "BasicCreator.h"

class BuildingCreatorUI
	: public BasicUI
{
public:
	virtual void draw() override;
};


class BuildingCreator
	: public BasicCreator<BuildingCreatorUI>
{
public:
	BuildingCreator(ObjectManager* objManager);
	void clickEvent();
};

