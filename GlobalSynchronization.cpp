#include "GlobalSynchronization.h"

namespace GlobalSynchronizaion
{
	detail::SynchronizationSet graphics = {};
	detail::SynchronizationSet physics	= {};
	//detail::SynchronizationSet window = {};

	std::condition_variable thread_cv = {};
	std::condition_variable main_cv = {};

	bool shouldStopEngine	= false;
}