#pragma once
#include <memory>
#include <condition_variable>
#include <shared_mutex>


namespace GlobalSynchronizaion
{
	namespace detail
	{
		struct SynchronizationSet
		{
			std::condition_variable cv = {};
			bool initialized = false;

			bool update		= false;
			bool updated	= false;

			bool cleanUp	= false;
			bool cleanedUp	= false;
		};
	}

	extern detail::SynchronizationSet graphics;
	extern detail::SynchronizationSet physics;
	extern detail::SynchronizationSet input;

	extern bool shouldStopEngine;
}