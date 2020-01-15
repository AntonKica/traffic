#include "House.h"
#include "Mesh.h"
#include <glm/gtc/constants.hpp>
void House::create(glm::vec3 position, std::string modelPath)
{
	Info::ModelInfo mInfo;
	mInfo.model = modelPath;

	setupModel(mInfo, true);

	getPhysicsComponent().updateSelfCollisionTags({ "BUILDING", "HOUSE" });
}
