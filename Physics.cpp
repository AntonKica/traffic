#include "Physics.h"
#include "PhysicsComponent.h"
#include "GlobalSynchronization.h"

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
	GlobalSynchronizaion::moduleInitialized = true;
	GlobalSynchronizaion::moduleInitialization.notify_one();

	while (GlobalSynchronizaion::shouldStopEngine == false)
	{
		{
			std::unique_lock<std::mutex> lock(m_updateMutex);
			GlobalSynchronizaion::engineUpdate.wait(lock, [] { return GlobalSynchronizaion::updateEngine; });
		}

		updateCollisions();

		// process collisions
		GlobalSynchronizaion::updatedPhysics = true;
		GlobalSynchronizaion::engineUpdate.notify_one();
	}
}

pPhysicsComponent Physics::createPhysicsComponent()
{
	return getPhysicsComponent();
}

pPhysicsComponent Physics::copyPhysicsComponent(const pPhysicsComponent& const copy)
{
	auto ptr = getPhysicsComponent();

	*ptr = *copy;
	return ptr;
}

void Physics::destroyPhysicsComponent(pPhysicsComponent& physicsComponent)
{
	*physicsComponent = {};

	m_activePhysicsComponents.erase(std::find(
		std::begin(m_activePhysicsComponents), 
		std::end(m_activePhysicsComponents), 
		physicsComponent));

	m_physicsComponents.push(physicsComponent);

	physicsComponent = nullptr;
}

pPhysicsComponent Physics::getPhysicsComponent()
{
	auto ptr = m_physicsComponents.top();
	m_physicsComponents.pop();

	m_activePhysicsComponents.emplace_back(ptr);
	return ptr;
}

void Physics::prepareResources()
{
	m_physicsComponentCount = 50'000;
	m_physicsComponentData = new PhysicsComponent[m_physicsComponentCount];

	for (uint32_t index = 0; index < m_physicsComponentCount; ++index)
		m_physicsComponents.push(m_physicsComponentData + index);
}

void Physics::destroyResourcces()
{
	m_physicsComponents = {};
	m_activePhysicsComponents = {};

	delete[]  m_physicsComponentData;
}

void Physics::updateCollisions()
{
	for (uint32_t currentIndex = 0; currentIndex < m_activePhysicsComponents.size(); ++currentIndex)
	{
		auto& currentComponent = *m_activePhysicsComponents[currentIndex];
		currentComponent.m_inCollision = false;
		for (uint32_t cursorIndex = currentIndex + 1; cursorIndex < m_activePhysicsComponents.size(); ++cursorIndex)
		{
			auto& cursorComponent = *m_activePhysicsComponents[cursorIndex];
			cursorComponent.m_inCollision = false;

			if (cursorComponent.collider2D.collides(currentComponent.collider2D))
			{
				currentComponent.m_inCollision = true;
				currentComponent.m_inCollision = true;
			}
		}
	}
}

bool Physics::finished() const
{
	return m_finished;
}
