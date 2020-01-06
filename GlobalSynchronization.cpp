#include "GlobalSynchronization.h"

namespace GlobalSynchronizaion
{
	detail::SynchronizationSet graphics = {};
	detail::SynchronizationSet physics	= {};
	detail::SynchronizationSet input	= {};

	bool shouldStopEngine	= false;
}