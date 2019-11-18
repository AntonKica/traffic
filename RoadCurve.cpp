#include "RoadCurve.h"

std::vector<EP::EntryPoint> RoadCurve::getEntryPoints() const
{
	int nOfRotations = m_rotation.x / 90.0;

	if (nOfRotations == 4)
		nOfRotations = 0;

	auto ep1 = EP::EntryPoint::RIGHT + nOfRotations;
	auto ep2 = ep1 + 1;

	return { ep1, ep2 };
}
/*
GridTile::ObjectType RoadCurve::getObjectType() const
{
	return GridTile::ObjectType::CURVE;
}

std::string RoadCurve::getTexturePath() const
{
	static std::string s_texturePath = "resources/materials/curved-road.png";
	return s_texturePath;
}
*/

std::string RoadCurve::getModelPath() const
{
	return std::string();
}

std::vector<Lane> RoadCurve::generateLanes()
{
	Path circularPath;
	float angle = 180.f;
	// weird fn
	int precision = 10;

	for (float alpha = 90; alpha <= angle; alpha += precision)
	{
		double pointX = std::cos(glm::radians(alpha)) + 1;
		double pointY = 1 - std::sin(glm::radians(alpha)) ;

		circularPath.addPoint({ pointX, pointY });
	}
	/*
	auto transformPoints = [](const glm::dvec2& point, double offset)
	{
		return point + offset;
	};
	*/
	double multiply = 0.75;
	std::vector<Lane> lanes(2);
	for (auto& lane : lanes)
	{
		for (const auto& circularPoint : circularPath.points)
		{
			auto newPoint = circularPoint * multiply;

			lane.points.push_back(newPoint);
		}
		multiply -= 0.5;
	}

	return lanes;
}

Path RoadCurve::getPath(bool rightLane)
{
	static std::vector<Path> paths = generateLanes();

	bool switchCoords = (m_rotation.x == 90.0 || m_rotation.x == 270.0);

	Lane retPath;
	for (const auto& lane : paths)
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
