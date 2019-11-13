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
#include "resource_creator.h"
#include "Grid.h"


#define DATA_FILE "DATA.dat"
constexpr uint32_t IDENTIFICATION_NUMBER = 0xAB'CD'EF'00;
using PositionRotation = std::pair<glm::dvec2, double>;

static std::pair<int,std::string> extractResourceData(std::istream& input)
{
	static const char* beginBracket = "{";
	static const char* endBracket = "}";

	std::optional<int> resourceType;
	std::string data;
	std::string in;
	bool foundBegin = false; 
	bool valid = false;
	while (input >> in)
	{
		// find resource type
		if (!resourceType.has_value())
		{
			resourceType = std::stoi(in);
			continue;
		}

		else if(in == beginBracket && !foundBegin)
		{
			foundBegin = true;
			continue;
		}
		else if (in == endBracket && resourceType.has_value())
		{
			valid = true;
			break;
		}

		data += in + " ";
	}

	std::pair<int, std::string> extractedData{};
	if (valid)
	{
		extractedData = std::make_pair(resourceType.value(), data);
	}

	return extractedData;
}

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
class scene
{
public:
	camera m_camera;
	resourceCreator& m_resourceCreator;
	Grid m_grid;
	scene()
		:m_resourceCreator(CreateResourceCreator())
	{
		m_camera = camera();

		auto initData = loadFromFile();
		m_grid.initGrid(initData);
	}
	~scene()
	{
		saveToFile();
	}

	std::map<GridTile::ObjectType, std::vector<PositionRotation>> loadFromFile()
	{
		std::map<GridTile::ObjectType, std::vector<PositionRotation>> positions;
		std::ifstream inputFile(DATA_FILE, std::ios::ate);

		if (!inputFile.is_open())
		{
			return {};
		}

		int fileSize = inputFile.tellg();
		if (fileSize == 0)
		{
			return{};
		}

		// reset cursor
		inputFile.seekg(0);

		uint32_t identificator;
		inputFile >> identificator;
		if (identificator != IDENTIFICATION_NUMBER)
		{
			return {};
		}

		inputFile.ignore(std::numeric_limits<int>().max(), '\n');

		while (!inputFile.eof())
		{
			auto[resType, posStr] = extractResourceData(inputFile);
			if (posStr.empty())
			{
				continue;
			}

			GridTile::ObjectType resourceType = static_cast<GridTile::ObjectType>(resType);
			positions[resourceType];

			std::string rawPositions = keepOnlyNumbers(posStr);

			std::stringstream stream(rawPositions);
			glm::dvec2 position;
			double rotation;
			while (stream >> position.x >> position.y >> rotation)
			{
				positions[resourceType].push_back(std::make_pair(position, rotation));
			}
		}

		return positions;
	}

	void saveToFile() const
	{
		std::ofstream inputFile(DATA_FILE, std::ios::trunc);

		inputFile << IDENTIFICATION_NUMBER << '\n';

		for (GridTile::ObjectType resType = static_cast<GridTile::ObjectType>(0);
			resType < GridTile::ObjectType::MAX_OBJECT_TYPES;
			++resType)
		{
			const auto objects = m_resourceCreator.getObjects(resType);
			if (objects.empty())
			{
				//nothin to copy
				continue;
			}

			inputFile << static_cast<int>(resType) << " { ";
			for (const auto& resource : objects)
			{
				glm::dvec3 position = resource->getWorldPosition();
				double rot = resource->getRotation();
				inputFile << " {" << position.x << ' ' << position.z  << ' ' << rot << "} ";
			}
			inputFile << " }\n";
		}
	}
};

#endif // !SCENE_H
