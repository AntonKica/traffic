#include "Physics.h"
#include "PhysicsComponent.h"
#include "GlobalSynchronization.h"
#include "SimulationObject.h"
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

		prepareFrame();
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
	*destinationPhysicsCore = *copyPhysicsCore;
	destinationPhysicsCore->pOwner = nullptr;
}

pPhysicsComponentCore Physics::copyCreatePhysicsComponentCore(const pPhysicsComponentCore& copyPhysicsCore)
{
	auto newCore = getPhysicsComponentCore();
	copyPhysicsComponentCore(copyPhysicsCore, newCore);

	return newCore;
}

void Physics::deactivatePhysicsComponentCore(pPhysicsComponentCore& physicsComponentCore)
{
	m_activePhysicsComponentCores.erase(
		std::find(std::begin(m_activePhysicsComponentCores), std::end(m_activePhysicsComponentCores), physicsComponentCore));

	m_physicsComponentCores.push(physicsComponentCore);

	*physicsComponentCore = {};
	physicsComponentCore = nullptr;
}


uint32_t Physics::getTagsFlag(const std::vector<std::string>& tagNames)
{
	uint32_t flag = 0;
	for (const auto& tagName : tagNames)
		flag |= getTagFlag(tagName);

	return flag;
}

uint32_t Physics::getTagFlag(const std::string& tagName)
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

	m_activePhysicsComponentCores.emplace_back(ptr);
	return ptr;
}

void Physics::prepareResources()
{
	m_physicsComponentCoreCount = 50'000;
	m_physicsComponentCoreData = new PhysicsComponentCore[m_physicsComponentCoreCount];

	// copy reverse
	for (int index = m_physicsComponentCoreCount - 1; index >= 0; --index)
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

void Physics::prepareFrame()
{
	// first count them
	uint32_t canBeDetectedCount = 0;
	uint32_t canDetectcount = 0;
	for (const auto& activeCore : m_activePhysicsComponentCores)
	{
		// skpi non active cores
		if (!activeCore->active)
			continue;

		// divide
		for (const auto& [_ ,collider]: activeCore->collider2Ds)
		{
			if (collider.hasSelfTags())
				++canBeDetectedCount;
			if (collider.hasOtherTags())
				++canDetectcount;
		}
	}

	{
		// clean from previous
		m_preparedColliders.canBeDetecdedByOthers.clear();
		m_preparedColliders.canDetectOthers.clear();

		// prepare space
		if (m_preparedColliders.canBeDetecdedByOthers.size() < canBeDetectedCount)
			m_preparedColliders.canBeDetecdedByOthers.resize(canBeDetectedCount);
		if (m_preparedColliders.canDetectOthers.size() < canDetectcount)
			m_preparedColliders.canDetectOthers.resize(canDetectcount);
	}

	// iterators
	auto canBeDetecdedByOthersIt = m_preparedColliders.canBeDetecdedByOthers.begin();
	auto canDetectOthersIt = m_preparedColliders.canDetectOthers.begin();

	// reset, add and categorize components
	for (auto& activeCore : m_activePhysicsComponentCores)
	{
		// skpi non active cores
		if (!activeCore->active)
			continue;

		for (auto& [_, collider] : activeCore->collider2Ds)
		{
			// reset the colider
			collider.clearCollisions();

			// assign
			AssociatedCollider associatedCollider = { activeCore->pOwner, &collider };

			if (collider.hasSelfTags())
				*canBeDetecdedByOthersIt++ = associatedCollider;
			if (collider.hasOtherTags())
				*canDetectOthersIt++ = associatedCollider;
		}
	}

	// goo to keep them sorted
	std::sort(m_preparedColliders.canBeDetecdedByOthers.begin(), m_preparedColliders.canBeDetecdedByOthers.end());
	std::sort(m_preparedColliders.canDetectOthers.begin(), m_preparedColliders.canDetectOthers.end());
}

void Physics::updateCollisions()
{
	for (auto& [firstOwner, firstCollider] :m_preparedColliders.canDetectOthers)
	{
		for (auto& [secondOwner, secondCollider] : m_preparedColliders.canBeDetecdedByOthers)
		{
			// dont try collision with same object
			if (firstOwner == secondOwner)
				continue;

			// check if we already collided with
			if(firstCollider->alreadyInCollisionWith(secondCollider))
				continue;

			//set first
			bool collided = false;
			if (firstCollider->canCollideWith(*secondCollider))
			{
				collided = firstCollider->collidesWith(*secondCollider);
				if (collided)
					firstCollider->addCollision(secondCollider, secondOwner);
			}

			// check if second also looks for same collison so we set it too
			if (collided)
			{
				if (secondCollider->canCollideWith(*firstCollider))
					secondCollider->addCollision(firstCollider, firstOwner);
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
