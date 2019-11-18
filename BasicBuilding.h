#pragma once
#include "SimulationAreaObjectStatic.h"
#include <string>

class BasicBuilding :
	public SimulationAreaObjectStatic
{
public:
	BasicBuilding();

protected:
	std::string getModelPath() const override;
//private:

};

