#pragma once
#include "BasicGeometry.h"
#include "PhysicsComponent.h"

#include <stack>
#include <mutex>

class VulkanBase;
class Physics
{
public:
	pPhysicsComponent createPhysicsComponent();
	pPhysicsComponent copyPhysicsComponent(const pPhysicsComponent& const copy);
	void destroyPhysicsComponent(pPhysicsComponent& physicsComponent);

	void run();
	void initialize();
	void mainLoop();

private:
	pPhysicsComponent getPhysicsComponent();

	void prepareResources();
	void destroyResourcces();

	void updateCollisions();

	PhysicsComponent* m_physicsComponentData;
	uint32_t m_physicsComponentCount;
	std::stack<PhysicsComponent*>  m_physicsComponents;
	std::vector<PhysicsComponent*> m_activePhysicsComponents;
	// synchronization
public:
	bool finished() const;
private:
	bool m_finished = false;
	std::mutex m_updateMutex;
};

