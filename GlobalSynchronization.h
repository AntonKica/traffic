#pragma once
#include <memory>
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace GlobalSynchronizaion
{
	namespace detail
	{
		struct SynchronizationSet
		{
			std::mutex notifyMutex;
			std::mutex updateWaitMutex;
			std::mutex cleanupWaitMutex;

			std::atomic<bool> initialized = false;

			std::atomic<bool> update	= false;
			std::atomic<bool> updated	= false;

			std::atomic<bool> cleanUp	= false;
			std::atomic<bool> cleanedUp	= false;
		};
	}

	extern detail::SynchronizationSet graphics;
	extern detail::SynchronizationSet physics;
	//extern detail::SynchronizationSet window;

	extern std::condition_variable thread_cv;
	extern std::condition_variable main_cv;

	extern std::atomic<bool> shouldStopEngine;
}