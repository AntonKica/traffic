#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"
#include "GlobalSynchronization.h"
#include "SimulationArea.h"

#include <boost/geometry.hpp>

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
			GlobalSynchronizaion::graphics.initialized.store(false);
			graphicsThread = std::thread(runGraphics);
			GlobalSynchronizaion::main_cv.wait(initializationLock, [] { return GlobalSynchronizaion::graphics.initialized.load(); });
		}

		{
			GlobalSynchronizaion::physics.initialized.store(false);
			physicsThread = std::thread(runPhysics);
			GlobalSynchronizaion::main_cv.wait(initializationLock, [] { return GlobalSynchronizaion::physics.initialized.load(); });
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

			// update wndow for curent frame
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
					GlobalSynchronizaion::graphics.update.store(true);
					GlobalSynchronizaion::graphics.updated.store(false);

					GlobalSynchronizaion::physics.update.store(true);
					GlobalSynchronizaion::physics.updated.store(false);

					GlobalSynchronizaion::thread_cv.notify_all();
				}
				// send and wait for signal
				{
					// we can update input meantime
					{
						App::input.update();
					}
					//std::cout << "Started all wait.. \n";
					auto wait = [] {
						return	!GlobalSynchronizaion::graphics.updated.load() ||
							!GlobalSynchronizaion::physics.updated.load();
					};

					while (wait())
						std::this_thread::sleep_for(std::chrono::nanoseconds(10));
				}
			}


			// update UI
			{
				UI::getInstance().updateDrawData();
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
			GlobalSynchronizaion::graphics.cleanUp.store(true);
			GlobalSynchronizaion::graphics.cleanedUp.store(false);
			GlobalSynchronizaion::thread_cv.notify_all();
			GlobalSynchronizaion::main_cv.wait(cleanUpLock, [] { return GlobalSynchronizaion::graphics.cleanedUp.load(); });
			graphicsThread.join();
		}

		{
			GlobalSynchronizaion::physics.cleanUp.store(true);
			GlobalSynchronizaion::physics.cleanedUp.store(false);
			GlobalSynchronizaion::thread_cv.notify_all();
			GlobalSynchronizaion::main_cv.wait(cleanUpLock, [] { return GlobalSynchronizaion::physics.cleanedUp.load(); });
			physicsThread.join();
		}
	}

	return 0;
}