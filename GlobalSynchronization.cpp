#include "GlobalSynchronization.h"

namespace GlobalSynchronizaion
{
	std::condition_variable moduleInitialization = {};
	bool moduleInitialized = false;

	std::condition_variable engineUpdate = {};
	bool updateEngine		= false;
	bool updatedGraphics	= false;
	bool updatedPhysics		= false;

	bool shouldStopEngine = false;
	//bool engineRunning = false;
}