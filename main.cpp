#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"

#include "GraphicsComponent.h"
void runGraphics(VulkanBase& base)
{
	try
	{
		base.run();
	}
	catch (const std::exception & exc)
	{
		std::cout << exc.what() << std::endl;
	}
}

int main()
{
	std::ios_base::sync_with_stdio(false);
	VulkanBase base;

	App::Scene.vulkanBase = &base;
	// experimental, might be unstable

	std::thread graphicsThread(runGraphics, std::ref(base));
	{
		static_assert("TU");
		std::cout << "NIeco fix alebo checknut, uz si nepamatam\n";
	}

	graphicsThread.join();
#ifndef _DEBUG
	std::cout << "Prss any button to exit... ";
	getchar();
#endif // _DEBUG

	return 0;
}