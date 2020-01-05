#pragma once
#include <memory>
#include <condition_variable>


namespace GlobalSynchronizaion
{
	extern std::condition_variable moduleInitialization;
	extern bool moduleInitialized;

	extern std::condition_variable engineUpdate;
	extern bool updateEngine;
	// modules 
	extern bool updatedGraphics;
	extern bool updatedPhysics;


	extern bool shouldStopEngine;
	//extern bool engineRunning;
}