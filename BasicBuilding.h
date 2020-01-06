#pragma once
#include "SimulationObject.h"
#include <string>

class BasicBuilding :
	public SimulationObject
{
public:
	enum class BuildingType
	{
		HOUSE,
		MAX_BUILDING_TYPE
	};

	BasicBuilding();

	virtual void create(glm::vec3 position, std::string modelPath) = 0;
protected:
	//std::string getModelPath() const override;
//private:

};

