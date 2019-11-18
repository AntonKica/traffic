#pragma once
#include "SimulationAreaObject.h"

namespace
{
	enum StaticAdjacencyBits
	{
		Neighbour	= 1 << 0,
		Distant		= 1 << 1,
		Diagonal	= 1 << 2,
		Axial		= 1 << 3,
		AxialX		= 1 << 4,
		AxialY		= 1 << 5,
		AxialZ		= 1 << 6,
	};
	typedef uint32_t StaticAdjacencyFlags;
}

class SimulationAreaObjectStatic :
	public SimulationAreaObject
{
protected:
	StaticAdjacencyFlags getObjectAdjacency(const SimulationAreaObjectStatic& other) const;
};

