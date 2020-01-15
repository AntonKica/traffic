#pragma once
#include "BasicGeometry.h"
#include "PhysicsInfo.h"
#include "PhysicsComponent.h"

#include <stack>
#include <unordered_map>

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

	void updatePhysicsComponentCollisionTags(pPhysicsComponentCore physicsCore, const Info::PhysicsComponentUpdateTags& updateInfo);

	uint32_t getTagFlag(std::string tagName);
private:
	pPhysicsComponentCore getPhysicsComponentCore();

	void prepareResources();
	void destroyResourcces();

	void updateCollisions();

	uint32_t createTagFlag(std::string tagName);
	bool compatibleTags(uint32_t firstFlags, uint32_t secondFlags) const;

	void registerPhysicsCoreTags(pPhysicsComponentCore& physicsCore, const std::vector<std::string>& tags);
	void registerPhysicsCoreTags(pPhysicsComponentCore& physicsCore, uint32_t newTag);
	void unregisterPhysicsCoreFromTags(pPhysicsComponentCore& physicsCore);

	pPhysicsComponentCore m_physicsComponentCoreData;
	uint32_t m_physicsComponentCoreCount;
	std::stack<pPhysicsComponentCore>  m_physicsComponentCores;


	std::unordered_map<uint32_t, std::vector<pPhysicsComponentCore>> m_activePhysicsComponentCores;
	std::unordered_map<std::string, uint32_t> m_tagFlags;
};

