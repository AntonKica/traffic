#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include <optional>

namespace Info
{
	struct PhysicsComponentUpdateTags
	{
		std::optional<std::vector<std::string>> newTags;
		std::optional<std::vector<std::string>> newOtherTags;
	};
}