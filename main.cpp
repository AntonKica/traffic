#include "VulkanBase.h"
#include <iostream>
#include <future>
#include "GlobalObjects.h"
#include "Collisions.h"

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
	Points pts1{ {-1.0, 0.0, 2.0}, {1.0, 0.0, 2.0} , {1.0, 0.0, -2.0} ,{-1.0, 0.0, -2.0} };
	for (auto& pt : pts1)
		pt.x += 4;
	Points pts2{ {-2.0, 0.0, 1.0}, {2.0, 0.0, 1.0} , {2.0, 0.0, -1.0} ,{-2.0, 0.0, -1.0} };
	for (auto& pt : pts2)
		pt.x += 4;
	bool colided = Collisions::polygonPolygonCollision(pts1, pts2);
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