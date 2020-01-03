#pragma once
#include "SimulationAreaObject.h"
#include <string>

class BasicBuilding :
	public SimulationAreaObject
{
public:
	BasicBuilding();

	virtual void create() = 0;
protected:
	//std::string getModelPath() const override;
//private:

};

