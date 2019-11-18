#include "BasicRoad.h"
#include "GlobalObjects.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <optional>
#include <algorithm>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>


bool EP::entryPointOpposite(const EP::EntryPoint& e1, const EP::EntryPoint& e2)
{
	// according to enum order
	return static_cast<int>(e1) % 2 == static_cast<int>(e2) % 2;
}

bool EP::entryPointOfXAxis(const EP::EntryPoint& e)
{
	return e == EP::EntryPoint::LEFT || e == EP::EntryPoint::RIGHT;
}
bool EP::entryPointOfZAxis(const EP::EntryPoint& e)
{
	return e == EP::EntryPoint::FRONT || e == EP::EntryPoint::BACK;
}

BasicRoad::BasicRoad()
{
	m_laneCount = 2;
	numberOfEntryPoints = 2;

	m_connections.clear();
	m_connections.reserve(numberOfEntryPoints);

	//createPath();
}

std::vector<EP::EntryPoint> BasicRoad::getEntryPoints() const
{
	if (m_rotation.x == 0 || m_rotation.x == 180)
		return { EP::EntryPoint::FRONT, EP::EntryPoint::BACK };
	else
		return { EP::EntryPoint::LEFT, EP::EntryPoint::RIGHT };
}


std::vector<Lane> BasicRoad::generateLanes()
{
	static const double laneWidth = 0.25;
	double roadWidth = m_laneCount * laneWidth;

	std::vector<Lane> lanes(m_laneCount);
	// generate lanes
	for (int n = 0; n < m_laneCount; ++n)
	{
		Lane newLane;
		double xPos = -(roadWidth / 2) + 2 * n * laneWidth;
		newLane.addPoints({ glm::dvec2(xPos, -0.5), glm::dvec2(xPos, 0.5) });

		lanes[n] = newLane;
	}

	return lanes;
}

Path BasicRoad::getPath(bool rightLane)
{
	// createLanes
	static std::vector<Lane> lanes = generateLanes();

	// take rotation into account
	bool switchCoords = (m_rotation.x == 180.0 || m_rotation.x == 180.0);

	Lane retPath;
	for (const auto& lane : lanes)
	{
		// useless
		if (rightLane && lane.points[0].x > 0)
		{
			retPath = lane;
			break;
		}
		else if (!rightLane)
		{
			retPath = lane;
			break;
		}
	}

	// transform to word coords
	auto transform = [&](const glm::dvec2& point)
	{
		auto pos = getPosition();
		auto newPoint = point;
		if (switchCoords)
			newPoint = glm::dvec2(point.y, point.x);

		return newPoint + glm::dvec2(pos.x, pos.z);
	};

	for (auto& point : retPath.points)
	{
		point = transform(point);
	}

	return retPath;
}

std::string BasicRoad::getModelPath() const
{
	static const std::string modelPath("resources/models/road/road.obj");

	return modelPath;
}


static bool adjacentPoints(glm::ivec2 firstPoint, glm::ivec2 secondPoint)
{
	glm::ivec2 diff = firstPoint - secondPoint;
	diff = { std::abs(diff.x), std::abs(diff.y) };

	return (diff.x <= 1 && diff.y <= 1);
}

/*
*   APPRECEATED
*	DONT USE
*	THANKS
*/
/*
void BasicRoad::createPath()
{
	static std::string s_Path = "resources/materials/roadPath.png";
	using pixel = unsigned char*;

	int width, height, channels;
	pixel data = stbi_load(s_Path.c_str(), &width, &height, &channels, 0);

	// analyza image

	std::vector<glm::ivec2> points;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int currentPixel = (x + height * y) * channels;

			pixel p = data + currentPixel;
									// A R G B
			static uint32_t white = 0xFF'FF'FF'FF;
			static uint32_t lessWhite = 0xFF'EE'EE'EE;

			uint32_t pixelColor = 0;
			for (int ch = 0; ch < channels; ++ch)
			{
				pixelColor |= uint32_t(*(p + ch)) << (ch * 8);
			}
			// add white pixel
			if (pixelColor <= white && pixelColor >= lessWhite)
				points.push_back({ x,y });
		}
	}
	// unthicken lines, later
	if (points.size() < 2)
		throw std::runtime_error("wtf, blba mapa");

	// getLine
	std::vector<std::vector<glm::ivec2>> lines;
	glm::ivec2 previousPoint;

	std::vector<glm::ivec2> newLine;


	int index = 0;
	while (!points.empty())
	{
		// if no beginPoint, assign
		if (newLine.empty())
		{
			previousPoint = points[0];
			points.erase(points.begin());

			newLine.push_back(previousPoint);
		}


		std::optional<glm::ivec2> nextPoint;
		for (auto point = points.begin(); point != points.end(); ++point)
		{
			if (adjacentPoints(previousPoint, *point))
			{
				nextPoint = *point;
				points.erase(point);

				break;
			}
		}

		// if found point
		if (nextPoint.has_value())
		{
			newLine.push_back(nextPoint.value());

			previousPoint = nextPoint.value();
		}
		else
		{
			// or not?
			if(newLine.size() > 1)
				lines.push_back(newLine);

			newLine.clear();
		}
	}
	// add last one
	if (newLine.size() > 1)
		lines.push_back(newLine);

	for (const auto& line : lines)
	{
		Path newPath;
		std::vector<glm::dvec2> points(2);
		points[0] = line.front();
		points[1] = line.back();

		newPath.addPoints(points);

	//	m_paths.push_back(newPath);
	}

}

std::vector<EP::EntryPoint> BasicRoad::getEP::EntryPoints()
{
	return std::vector<EP::EntryPoint>();
}
*/

void BasicRoad::connectRoads(BasicRoad& lhs, BasicRoad& rhs)
{
	if (alreadyConnected(lhs, rhs) || !canConnect(lhs, rhs))
	{
		return;
	}
	auto lhsEntryPoints = lhs.getEntryPoints();
	auto rhsEntryPoints = rhs.getEntryPoints();
	glm::ivec3 diff = rhs.getPosition() - lhs.getPosition();

	Connection lhsConnection{ &rhs, {} };
	Connection rhsConnection{ &lhs, {} };

	StaticAdjacencyFlags adjacency = lhs.getObjectAdjacency(rhs);
	if ((adjacency & StaticAdjacencyBits::Diagonal) == Diagonal)
	{
		throw;
	}

	static int conncount = 0;
	if (adjacency & StaticAdjacencyBits::AxialX)
	{
		if (diff.x == -1)
		{
			lhsConnection.entryPoint = EP::EntryPoint::LEFT;
			rhsConnection.entryPoint = EP::EntryPoint::RIGHT;
		}
		else if (diff.x == 1)
		{
			lhsConnection.entryPoint = EP::EntryPoint::RIGHT;
			rhsConnection.entryPoint = EP::EntryPoint::LEFT;
		}

		lhs.m_connections.push_back(lhsConnection);
		rhs.m_connections.push_back(rhsConnection);
		std::cout << "Total connections = " << ++conncount << '\n';
	}
	else if (adjacency & StaticAdjacencyBits::AxialZ)
	{
		if (diff.z == -1)
		{
			lhsConnection.entryPoint = EP::EntryPoint::FRONT;
			rhsConnection.entryPoint = EP::EntryPoint::BACK;
		}
		else
		{
			lhsConnection.entryPoint = EP::EntryPoint::BACK;
			rhsConnection.entryPoint = EP::EntryPoint::FRONT;
		}
		lhs.m_connections.push_back(lhsConnection);
		rhs.m_connections.push_back(rhsConnection);
		std::cout << "Total connections = " << ++conncount << '\n';
	}
}

bool BasicRoad::canConnect(const BasicRoad& lhs, const BasicRoad& rhs)
{
	//check for diagonal
	StaticAdjacencyFlags adjacency = lhs.getObjectAdjacency(rhs);
	StaticAdjacencyFlags restriction = StaticAdjacencyBits::Diagonal;

	if ((adjacency & restriction) == restriction)
	{
		return false;
	}
	glm::ivec3 diff = glm::abs(lhs.getPosition() - rhs.getPosition());

	auto lhsEntryPoints = lhs.getEntryPoints();
	auto rhsEntryPoints = rhs.getEntryPoints();

	// zbytocne?
	for (const auto lhsEntryPoint : lhsEntryPoints)
	{
		for (const auto rhsEntryPoint : rhsEntryPoints)
		{
			if (entryPointOpposite(lhsEntryPoint, rhsEntryPoint))
			{
				if ((diff.x != 0 && entryPointOfXAxis(lhsEntryPoint)) ||
					(diff.z != 0 && entryPointOfZAxis(lhsEntryPoint)))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool BasicRoad::alreadyConnected(const BasicRoad& lhs, const BasicRoad& rhs)
{
	if (lhs.m_connections.empty() || rhs.m_connections.empty())
	{
		return false;
	}
	else
	{
		for (const auto& connection : lhs.m_connections)
		{
			if (connection.connected == &rhs)
				return true;
		}
	}
	return false;
}

