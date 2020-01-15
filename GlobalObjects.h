#pragma once
#include "Time.h"
#include "camera.h"
#include "VulkanBase.h"
#include "Physics.h"
#include "Simulation.h"
#include "Input.h"
#include "Window.h"
//namespace
namespace App
{
	extern Window window;
	extern Camera camera;
	extern Time time;
	extern VulkanBase vulkanBase;
	extern Physics physics;
	extern Input input;
	extern Simulation simulation;
}

