#ifndef SCENE_H
#define SCENE_H

#include <fstream>
#include <algorithm>
#include <utility>
#include <optional>
#include <sstream>
#include <functional>

#include <glm/gtx/string_cast.hpp>

#include "camera.h"
#include "Time.h"
#include "SimulationArea.h"
#include "VulkanBase.h"


#define DATA_FILE "DATA.dat"
constexpr uint32_t IDENTIFICATION_NUMBER = 0xAB'CD'EF'00;

static std::string keepOnlyNumbers(const std::string& str)
{
	static const std::string plausible = { ".-+0123456789 " };

	std::string newStr;
	for (const char& c : str)
	{
		if (std::find(plausible.begin(), plausible.end(), c) != plausible.end())
			newStr += c;
	}

	return newStr;
}
class Scene
{
public:
	// dont change order, stack favours this :)
	camera m_camera;
	Time time;
	VulkanBase vulkanBase;
	SimulationArea m_simArea;

	Scene()
	{
		m_camera = camera();
	}
	~Scene()
	{
		///saveToFile();
	}

	// another ugly
	void initComponents()
	{
		m_simArea.initArea();
	}
};

#endif // !SCENE_H
