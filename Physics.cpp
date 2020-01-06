#include "Physics.h"
#include "PhysicsComponent.h"
#include "GlobalSynchronization.h"

#include <iostream>

void Physics::run()
{
	initialize();

	mainLoop();

	destroyResourcces();
}

void Physics::initialize()
{
	prepareResources();
}

void Physics::mainLoop()
{
	GlobalSynchronizaion::physics.initialized = true;
	GlobalSynchronizaion::physics.cv.notify_one();

	while (GlobalSynchronizaion::shouldStopEngine == false)
	{
		{
			std::mutex updateWaitMutex;
			std::unique_lock<std::mutex> updateWaitLock(updateWaitMutex);
			GlobalSynchronizaion::physics.cv.wait(updateWaitLock, [] { return GlobalSynchronizaion::physics.update; });
			GlobalSynchronizaion::physics.update = false;
		}

		updateCollisions();

		// process collisions
		{
			GlobalSynchronizaion::physics.updated = true;
			GlobalSynchronizaion::physics.cv.notify_one();
		}
	}
}

pPhysicsComponentCore Physics::createPhysicsComponentCore()
{
	return getPhysicsComponentCore();
}

void Physics::copyPhysicsComponentCore(const pPhysicsComponentCore& copyPhysicsCore, pPhysicsComponentCore& destinationPhysicsCore)
{
	*destinationPhysicsCore = *copyPhysicsCore;
}

pPhysicsComponentCore Physics::copyCreatePhysicsComponentCore(const pPhysicsComponentCore& copyPhysicsCore)
{
	auto newCore = getPhysicsComponentCore();
	copyPhysicsComponentCore(copyPhysicsCore, newCore);

	return newCore;
}

void Physics::deactivatePhysicsComponentCore(pPhysicsComponentCore& physicsComponentCore)
{

	m_activePhysicsComponentCores.erase(std::find(
		std::begin(m_activePhysicsComponentCores),
		std::end(m_activePhysicsComponentCores),
		physicsComponentCore));

	m_physicsComponentCores.push(physicsComponentCore);

	*physicsComponentCore = {};
	physicsComponentCore = nullptr;
}

pPhysicsComponentCore Physics::getPhysicsComponentCore()
{
	auto ptr = m_physicsComponentCores.top();
	m_physicsComponentCores.pop();

	m_activePhysicsComponentCores.emplace_back(ptr);
	return ptr;
}

void Physics::prepareResources()
{
	m_physicsComponentCoreCount = 50'000;
	m_physicsComponentCoreData = new PhysicsComponentCore[m_physicsComponentCoreCount];

	for (uint32_t index = 0; index < m_physicsComponentCoreCount; ++index)
		m_physicsComponentCores.push(m_physicsComponentCoreData + index);
}

void Physics::destroyResourcces()
{
	{
		std::mutex cleanupMutex;
		std::unique_lock<std::mutex> cleanupLock(cleanupMutex);
		GlobalSynchronizaion::physics.cv.wait(cleanupLock, [] { return GlobalSynchronizaion::physics.cleanUp; });
		GlobalSynchronizaion::physics.cleanUp = false;
	}

	m_physicsComponentCores = {};
	m_activePhysicsComponentCores = {};

	delete[]  m_physicsComponentCoreData;

	{
		GlobalSynchronizaion::physics.cleanedUp = true;
		GlobalSynchronizaion::physics.cv.notify_one();
	}
}

void Physics::updateCollisions()
{
	for (uint32_t currentIndex = 0; currentIndex < m_activePhysicsComponentCores.size(); ++currentIndex)
	{
		auto& currentCore = m_activePhysicsComponentCores[currentIndex];
		if (!currentCore->active)
			continue;
		
		currentCore->inCollision = false;
		for (uint32_t cursorIndex = currentIndex + 1; cursorIndex < m_activePhysicsComponentCores.size(); ++cursorIndex)
		{
			auto& cursorCore = m_activePhysicsComponentCores[cursorIndex];
			if (!cursorCore->active)
				continue;

			cursorCore->inCollision = false;

			if (cursorCore->collider2D.collides(currentCore->collider2D))
			{
				cursorCore->inCollision = true;
				currentCore->inCollision = true;
			}
		}
	}
}
