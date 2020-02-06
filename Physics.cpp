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
	{
		std::lock_guard notifyLock(GlobalSynchronizaion::physics.notifyMutex);

		GlobalSynchronizaion::physics.initialized.store(true);
		GlobalSynchronizaion::main_cv.notify_one();
	}
	while (GlobalSynchronizaion::shouldStopEngine.load() == false)
	{
		{
			//std::cout << "P wait\n";
			std::unique_lock updateWaitLock(GlobalSynchronizaion::physics.updateWaitMutex);

			auto waitPred = [] { return GlobalSynchronizaion::physics.update.load(); };
			while (!waitPred())
				GlobalSynchronizaion::thread_cv.wait(updateWaitLock);

			GlobalSynchronizaion::physics.update.store(false);
		}

		updateCollisions();

		{
			//std::cout << "P finished\n";
			std::lock_guard notifyLock(GlobalSynchronizaion::physics.notifyMutex);

			GlobalSynchronizaion::physics.updated.store(true);
			GlobalSynchronizaion::main_cv.notify_one();
		}
	}
}

pPhysicsComponentCore Physics::createPhysicsComponentCore()
{
	return getPhysicsComponentCore();
}

void Physics::copyPhysicsComponentCore(const pPhysicsComponentCore& copyPhysicsCore, pPhysicsComponentCore& destinationPhysicsCore)
{
	unregisterPhysicsCoreFromTags(destinationPhysicsCore);

	*destinationPhysicsCore = *copyPhysicsCore;
	destinationPhysicsCore->pOwner = nullptr;
	// register raw
	registerPhysicsCoreTags(destinationPhysicsCore, copyPhysicsCore->tag);
}

pPhysicsComponentCore Physics::copyCreatePhysicsComponentCore(const pPhysicsComponentCore& copyPhysicsCore)
{
	auto newCore = getPhysicsComponentCore();
	copyPhysicsComponentCore(copyPhysicsCore, newCore);

	return newCore;
}

void Physics::deactivatePhysicsComponentCore(pPhysicsComponentCore& physicsComponentCore)
{
	unregisterPhysicsCoreFromTags(physicsComponentCore);

	m_physicsComponentCores.push(physicsComponentCore);

	*physicsComponentCore = {};
	physicsComponentCore = nullptr;
}

void Physics::setPhysicsComponentCollisionTags(pPhysicsComponentCore physicsCore, const Info::PhysicsComponentUpdateTags& updateInfo)
{
	if (updateInfo.newTags)
	{
		unregisterPhysicsCoreFromTags(physicsCore);
		registerPhysicsCoreTags(physicsCore, updateInfo.newTags.value());
	}

	// assingon other tags
	if (updateInfo.newOtherTags)
	{
		physicsCore->otherTags = 0;
		for (const auto& newOtherTag : updateInfo.newOtherTags.value())
			physicsCore->otherTags |= getTagFlag(newOtherTag);
	}
}

uint32_t Physics::getTagFlag(std::string tagName)
{
	auto optFlag = m_tagFlags.find(tagName);
	if (optFlag != m_tagFlags.end())
		return optFlag->second;
	else
		return createTagFlag(tagName);
}

bool Physics::compatibleTags(uint32_t firstFlags, uint32_t secondFlags) const
{
	return (firstFlags & secondFlags) != 0;
}


pPhysicsComponentCore Physics::getPhysicsComponentCore()
{
	auto ptr = m_physicsComponentCores.top();
	m_physicsComponentCores.pop();

	m_activePhysicsComponentCores[0].emplace_back(ptr);
	return ptr;
}

void Physics::prepareResources()
{
	m_physicsComponentCoreCount = 50'000;
	m_physicsComponentCoreData = new PhysicsComponentCore[m_physicsComponentCoreCount];

	for (uint32_t index = 0; index < m_physicsComponentCoreCount; ++index)
		m_physicsComponentCores.push(m_physicsComponentCoreData + index);

	// tag flag default
	// so we have at least one string tag
	m_tagFlags[std::string()] = 0;
}

void Physics::destroyResourcces()
{
	{
		std::unique_lock cleanupWaitLock(GlobalSynchronizaion::physics.cleanupWaitMutex);
		GlobalSynchronizaion::thread_cv.wait(cleanupWaitLock, [] { return GlobalSynchronizaion::physics.cleanUp.load(); });
		GlobalSynchronizaion::physics.cleanUp.store(false);
	}

	m_physicsComponentCores = {};
	m_activePhysicsComponentCores = {};

	delete[]  m_physicsComponentCoreData;

	{
		std::lock_guard notifyLock(GlobalSynchronizaion::physics.notifyMutex);

		GlobalSynchronizaion::physics.cleanedUp.store(true);
		GlobalSynchronizaion::main_cv.notify_one();
	}
}

void Physics::updateCollisions()
{
	if (m_tagFlags.size() <= 1)
		return;

	for (auto& [_, comps] : m_activePhysicsComponentCores)
	{
		for (auto& comp : comps)
		{
			// dont clear whole map
			for (auto& [tag, objs] : comp->inCollisionWith)
			{
				objs.clear();
			}
		}
	}

	// process by tags
	for (auto& [firstTag, firstComponents] : m_activePhysicsComponentCores)
	{
		// comopnent tagged with 0 dont collide
		if (firstTag == 0)
			continue;
		// each component
		for (auto& firstComponent : firstComponents)
		{

			// or if no othr collisions skip
			if (!firstComponent->active || !firstComponent->otherTags)
				continue;

			// otherwise check
			for (auto& [secondTag, secondComponents] : m_activePhysicsComponentCores)
			{
				// if can collide with this flags
				if (!compatibleTags(firstComponent->otherTags, secondTag))
					continue;

				// optherwise do collision wit each other component
				for (auto& secondComponent : secondComponents)
				{
					if (!secondComponent->active || &firstComponent == &secondComponent)
						continue;

					if (firstComponent->collider2D.collides(secondComponent->collider2D))
					{
						firstComponent->inCollisionWith [secondTag].emplace_back(secondComponent->pOwner);
					}
				}
			}
		}
	}
}

uint32_t Physics::createTagFlag(std::string tagName)
{
	// register
	uint32_t uniqueFlag = static_cast<uint32_t>(1) << static_cast<uint32_t>(m_tagFlags.size() - 1);
	m_tagFlags[tagName] = uniqueFlag;

	return uniqueFlag;
}

void Physics::registerPhysicsCoreTags(pPhysicsComponentCore& physicsCore, const std::vector<std::string>& tags)
{
	uint32_t newTag = 0;
	for (const auto& newTagFlag : tags)
		newTag |= getTagFlag(newTagFlag);

	physicsCore->tag = newTag;
	m_activePhysicsComponentCores[newTag].emplace_back(physicsCore);
}

void Physics::registerPhysicsCoreTags(pPhysicsComponentCore& physicsCore, uint32_t newTag)
{
	physicsCore->tag = newTag;
	m_activePhysicsComponentCores[newTag].emplace_back(physicsCore);
}

void Physics::unregisterPhysicsCoreFromTags(pPhysicsComponentCore& physicsCore)
{
	auto& [_, cores] = *m_activePhysicsComponentCores.find(physicsCore->tag);
	cores.erase(std::find(std::begin(cores), std::end(cores), physicsCore));

	physicsCore->tag = 0;
}
