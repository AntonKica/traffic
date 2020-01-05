#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"
#include "GlobalSynchronization.h"
#include "Collisions.h"

#include "GraphicsComponent.h"
#include "VulkanDataManager.h"

void runGraphics()
{
	try
	{
		App::vulkanBase.run();
	}
	catch (const std::exception & exc)
	{
		std::cout << exc.what() << std::endl;
	}
}

void runPhysics()
{
	try
	{
		App::physics.run();
	}
	catch (const std::exception & exc)
	{
		std::cout << exc.what() << std::endl;
	}
}

int main()
{
	//throw std::runtime_error("Pri vymazani treaba vymazat aj zo vsetkych cast, lebo DescrutporSet sa preplnuje!");
	std::ios_base::sync_with_stdio(false);
	// experimental, might be unstable

	std::thread graphicsThread;
	std::thread physicsThread;

	//GlobalSynchronizaion::shouldStopEngine = false;
	// initialize
	{
		std::mutex initializationMutex;
		std::unique_lock initializationLock(initializationMutex);

		{
			GlobalSynchronizaion::moduleInitialized = false;
			graphicsThread = std::thread(runGraphics);
			GlobalSynchronizaion::moduleInitialization.wait(initializationLock, [] { return GlobalSynchronizaion::moduleInitialized; });
		}

		{
			GlobalSynchronizaion::moduleInitialized = false;
			graphicsThread = std::thread(runPhysics);
			GlobalSynchronizaion::moduleInitialization.wait(initializationLock, [] { return GlobalSynchronizaion::moduleInitialized; });
		}
	}

	GlobalSynchronizaion::shouldStopEngine = true;
	while(GlobalSynchronizaion::shouldStopEngine == false)
	{
		// update graphics and physics
		std::mutex updateMutex;
		std::unique_lock updateLock(updateMutex);

		GlobalSynchronizaion::updateEngine = true;
		GlobalSynchronizaion::engineUpdate.notify_all();
		GlobalSynchronizaion::engineUpdate.wait(updateLock, 
			[]	{	return GlobalSynchronizaion::updatedGraphics && GlobalSynchronizaion::updatedPhysics; });

		// update objects


		GlobalSynchronizaion::shouldStopEngine = glfwWindowShouldClose(App::vulkanBase.getWindow());
	}
	//GlobalSynchronizaion::engineRunning = false;

	physicsThread.join();
	graphicsThread.join();
#ifndef _DEBUG
	std::cout << "Prss any button to exit... ";
	getchar();
#endif // _DEBUG

	return 0;
}