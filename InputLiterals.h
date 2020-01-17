#pragma once
#include <stdint.h>

namespace InputLiterals
{
	using GLFW_VALUE = uint32_t;
	using GLFW_KEYCODE = uint16_t;
	using GLFW_KEY_STATE = GLFW_VALUE;

	struct Record
	{
		GLFW_KEY_STATE pressed = false;

		bool pressedCurrentFrame = false;
		bool pressedLastFrame = false;
	};
}
namespace IL = InputLiterals;