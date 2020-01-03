#include "House.h"
#include "Mesh.h"
#include <glm/gtc/constants.hpp>
void House::create()
{
	Info::ModelInfo mInfo;
	mInfo.model = "resources/models/house/house.obj";

	setupModel(mInfo, true);
}
