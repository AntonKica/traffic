#pragma once
#include "BasicGeometry.h"
#include "PhysicsComponent.h"

#include <stack>
#include <mutex>

class VulkanBase;
class Physics
{
public:
	void run();
	void initialize();
	void mainLoop();

	pPhysicsComponentCore createPhysicsComponentCore();
	void copyPhysicsComponentCore(const pPhysicsComponentCore& copyPhysicsCore, pPhysicsComponentCore& destinationPhysicsCore);
	pPhysicsComponentCore copyCreatePhysicsComponentCore(const pPhysicsComponentCore& copyPhysicsCore);
	void deactivatePhysicsComponentCore(pPhysicsComponentCore& physicsComponent);
private:
	pPhysicsComponentCore getPhysicsComponentCore();

	void prepareResources();
	void destroyResourcces();

	void updateCollisions();

	pPhysicsComponentCore m_physicsComponentCoreData;
	uint32_t m_physicsComponentCoreCount;
	std::stack<pPhysicsComponentCore>  m_physicsComponentCores;
	std::vector<pPhysicsComponentCore> m_activePhysicsComponentCores;
};

