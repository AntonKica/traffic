#pragma once
#include <memory>
#include <condition_variable>
#include <mutex>


namespace GlobalSynchronizaion
{
	namespace detail
	{
		struct SynchronizationSet
		{
			std::mutex notifyMutex;
			std::mutex updateWaitMutex;
			std::mutex cleanupWaitMutex;

			bool initialized = false;

			bool update		= false;
			bool updated	= false;

			bool cleanUp	= false;
			bool cleanedUp	= false;
		};
	}

	extern detail::SynchronizationSet graphics;
	extern detail::SynchronizationSet physics;
	//extern detail::SynchronizationSet window;

	extern std::condition_variable thread_cv;
	extern std::condition_variable main_cv;

	extern bool shouldStopEngine;
}