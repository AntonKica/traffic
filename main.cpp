#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"
#include "GlobalSynchronization.h"
#include "SimulationArea.h"

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

void runInput()
{
	try
	{
		App::input.run();
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
	std::thread inputThread;

	//GlobalSynchronizaion::shouldStopEngine = false;
	// initialize
	{
		std::mutex initializationMutex;
		std::unique_lock initializationLock(initializationMutex);

		{
			GlobalSynchronizaion::graphics.initialized = false;
			graphicsThread = std::thread(runGraphics);
			GlobalSynchronizaion::graphics.cv.wait(initializationLock, [] { return GlobalSynchronizaion::graphics.initialized; });
		}

		{
			GlobalSynchronizaion::physics.initialized = false;
			physicsThread = std::thread(runPhysics);
			GlobalSynchronizaion::physics.cv.wait(initializationLock, [] { return GlobalSynchronizaion::physics.initialized; });
		}

		{
			GlobalSynchronizaion::input.initialized = false;
			inputThread = std::thread(runInput);
			GlobalSynchronizaion::input.cv.wait(initializationLock, [] { return GlobalSynchronizaion::input.initialized; });
		}
	}

	// Input
	{
		App::input.initialize();
	}

	{
		App::time.tick();
		SimulationArea simulationArea;
		simulationArea.initArea();

		GlobalSynchronizaion::shouldStopEngine = false;
		while (GlobalSynchronizaion::shouldStopEngine == false)
		{
			App::time.tick();

			// firstly update camera
			{
				App::camera.update();
			}
			// should before update, because they rely on this
			GlobalSynchronizaion::shouldStopEngine = glfwWindowShouldClose(App::vulkanBase.getWindow());
			// update graphics and physics
			{
				std::mutex updateMutex;
				std::unique_lock updateLock(updateMutex);

				GlobalSynchronizaion::graphics.update = true;
				GlobalSynchronizaion::physics.update = true;
				GlobalSynchronizaion::graphics.cv.notify_one();
				GlobalSynchronizaion::physics.cv.notify_one();

				GlobalSynchronizaion::graphics.cv.wait(updateLock, []
					{ return GlobalSynchronizaion::graphics.updated; });
				GlobalSynchronizaion::graphics.updated = false;
				GlobalSynchronizaion::physics.cv.wait(updateLock, []
					{ return GlobalSynchronizaion::physics.updated; });
				GlobalSynchronizaion::physics.updated = false;

				GlobalSynchronizaion::input.update = true;
				GlobalSynchronizaion::input.cv.notify_one();
				GlobalSynchronizaion::input.cv.wait(updateLock, []
					{ return GlobalSynchronizaion::input.updated; });
				GlobalSynchronizaion::input.updated = false;
			}

			{
				// update objects
				App::simulation.updateSimulation();
				simulationArea.update();
			}
		}
	}

	{
		std::mutex cleanUpMutex;
		std::unique_lock cleanUpLock(cleanUpMutex);
		{
			GlobalSynchronizaion::graphics.cleanUp = true;
			GlobalSynchronizaion::graphics.cleanedUp = false;
			GlobalSynchronizaion::graphics.cv.notify_one();
			GlobalSynchronizaion::graphics.cv.wait(cleanUpLock, [] { return GlobalSynchronizaion::graphics.cleanedUp; });
			graphicsThread.join();
		}

		{
			GlobalSynchronizaion::physics.cleanUp = true;
			GlobalSynchronizaion::physics.cleanedUp = false;
			GlobalSynchronizaion::physics.cv.notify_one();
			GlobalSynchronizaion::physics.cv.wait(cleanUpLock, [] { return GlobalSynchronizaion::physics.cleanedUp; });
			physicsThread.join();
		}

		{
			GlobalSynchronizaion::input.cleanUp = true;
			GlobalSynchronizaion::input.cleanedUp = false;
			GlobalSynchronizaion::input.cv.notify_one();
			GlobalSynchronizaion::input.cv.wait(cleanUpLock, [] { return GlobalSynchronizaion::input.cleanedUp; });
			inputThread.join();
		}
	}

#ifndef _DEBUG
	std::cout << "Prss any button to exit... ";
	getchar();
#endif // _DEBUG

	return 0;
}