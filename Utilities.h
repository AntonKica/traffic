#pragma once

namespace Utility
{
	class NonCopy
	{
	public:
		NonCopy() = default;
		NonCopy(const NonCopy&) = delete;
		NonCopy(const NonCopy&&) = delete;
		NonCopy& operator= (const NonCopy&) = delete;
		NonCopy& operator= (const NonCopy&&) = delete;
	};

	template<class T>
	size_t generateNextContainerID(T c)
	{
		return c.size();
	}
}