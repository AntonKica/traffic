#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"

#include "GraphicsComponent.h"
#include "VulkanDataManager.h"

#include "Model.h"
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
	// experimental, might be unstable

	std::thread graphicsThread(runGraphics, std::ref(App::Scene.vulkanBase));

	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		Model model("resources/models/house/house.obj");

		VulkanDataManager dm;
		dm.initialize(&App::Scene.vulkanBase);

		dm.loadModel(model);
		return 0;
	}

	graphicsThread.join();
#ifndef _DEBUG
	std::cout << "Prss any button to exit... ";
	getchar();
#endif // _DEBUG

	return 0;
}