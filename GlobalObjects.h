#pragma once
#include "Time.h"
#include "camera.h"
#include "VulkanBase.h"
#include "Physics.h"
#include "SimulationArea.h"

//namespace
namespace App
{
	extern Camera camera;
	extern Time time;
	extern VulkanBase vulkanBase;
	extern Physics physics;
	extern SimulationArea simulationArea;
}

