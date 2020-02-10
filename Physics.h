#pragma once
#include "BasicGeometry.h"
#include "PhysicsInfo.h"
#include "PhysicsComponent.h"
#include "GraphicsComponent.h"

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

	uint32_t getTagsFlag(const std::vector<std::string>& tagNames);
	uint32_t getTagFlag(const std::string& tagName);
	bool compatibleTags(uint32_t firstFlags, uint32_t secondFlags) const;
private:
	pPhysicsComponentCore getPhysicsComponentCore();

	void prepareResources();
	void destroyResourcces();

	void prepareFrame();
	void updateCollisions();

	uint32_t createTagFlag(std::string tagName);
	
	pPhysicsComponentCore m_physicsComponentCoreData;
	uint32_t m_physicsComponentCoreCount;
	std::stack<pPhysicsComponentCore>  m_physicsComponentCores;

	std::vector<pPhysicsComponentCore> m_activePhysicsComponentCores;
	std::unordered_map<std::string, uint32_t> m_tagFlags;

	struct AssociatedCollider
	{
		SimulationObject* simObject;
		Collider2D* collider;

		// sorti via collider tags
		bool operator<(const AssociatedCollider& other) const
		{
			return collider->m_tags < other.collider->m_tags;
		}
	};
	struct
	{
		std::vector<AssociatedCollider> canBeDetecdedByOthers;
		std::vector<AssociatedCollider> canDetectOthers;
	} m_preparedColliders;
};

