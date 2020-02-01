#pragma once
#include "BasicBuilding.h"
class House
	: public BasicBuilding
{
public:
	virtual void create(glm::vec3 position, std::string modelPath);
};

