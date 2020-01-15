#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"
#include "GlobalSynchronization.h"
#include "SimulationArea.h"

constexpr const char* AppName = "Traffic Simulation";

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
	//std::thread windowThread;

	{
		App::window.initialize(AppName);
	}
	{
		std::mutex initializationMutex;
		std::unique_lock initializationLock(initializationMutex);

		{
			GlobalSynchronizaion::graphics.initialized = false;
			graphicsThread = std::thread(runGraphics);
			GlobalSynchronizaion::main_cv.wait(initializationLock, [] { return GlobalSynchronizaion::graphics.initialized; });
		}

		{
			GlobalSynchronizaion::physics.initialized = false;
			physicsThread = std::thread(runPhysics);
			GlobalSynchronizaion::main_cv.wait(initializationLock, [] { return GlobalSynchronizaion::physics.initialized; });
		}
	}

	{
		App::time.tick();
		SimulationArea simulationArea;
		simulationArea.initArea();

		GlobalSynchronizaion::shouldStopEngine = false;
		while (GlobalSynchronizaion::shouldStopEngine == false)
		{
			//std::cout << "New loop\n";
			App::time.tick();

			// update outside objects
			{
				App::window.updateFrame();
				App::camera.update();
			}
			// should before update, because they rely on this
			GlobalSynchronizaion::shouldStopEngine = App::window.isClosed();
			// update graphics and physics
			{
				std::mutex updateMutex;
				std::unique_lock updateLock(updateMutex);

				// reset
				{
					GlobalSynchronizaion::graphics.update = true;
					GlobalSynchronizaion::graphics.updated = false;

					GlobalSynchronizaion::physics.update = true;
					GlobalSynchronizaion::physics.updated = false;

					GlobalSynchronizaion::thread_cv.notify_all();
				}
				// send and wait for signal
				{

					//std::cout << "Started all wait.. \n";
					auto waitPred = [] {
						return	GlobalSynchronizaion::graphics.updated &&
							GlobalSynchronizaion::physics.updated;
					};

					while (!waitPred())
						//GlobalSynchronizaion::main_cv.wait_for(updateLock, std::chrono::milliseconds(1));
						std::this_thread::sleep_for(std::chrono::nanoseconds(10));
				}
			}

			{
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
			GlobalSynchronizaion::thread_cv.notify_all();
			GlobalSynchronizaion::main_cv.wait(cleanUpLock, [] { return GlobalSynchronizaion::graphics.cleanedUp; });
			graphicsThread.join();
		}

		{
			GlobalSynchronizaion::physics.cleanUp = true;
			GlobalSynchronizaion::physics.cleanedUp = false;
			GlobalSynchronizaion::thread_cv.notify_all();
			GlobalSynchronizaion::main_cv.wait(cleanUpLock, [] { return GlobalSynchronizaion::physics.cleanedUp; });
			physicsThread.join();
		}
	}

	return 0;
}