#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"

#include "GraphicsComponent.h"
#include "VulkanDataManager.h"

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
	//throw std::runtime_error("Pri vymazani treaba vymazat aj zo vsetkych cast, lebo DescrutporSet sa preplnuje!");
	std::ios_base::sync_with_stdio(false);
	// experimental, might be unstable

	std::thread graphicsThread(runGraphics, std::ref(App::Scene.vulkanBase));

	graphicsThread.join();
#ifndef _DEBUG
	std::cout << "Prss any button to exit... ";
	getchar();
#endif // _DEBUG

	return 0;
}