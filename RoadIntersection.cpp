#include "RoadIntersection.h"
#include "Utilities.h"
#include <glm/gtx/string_cast.hpp>
#include <numeric>

inline VD::Indices createPseudoTriangleFanIndices(const Points& points)
{
	VD::Indices indices;

	int maxSize = points.size();
	for (int i = 1; i < maxSize; ++i)
	{
		std::vector<uint32_t> fanIndices
		{
			0, 
			static_cast<uint32_t>(i), 
			static_cast<uint32_t>((i + 1 == maxSize ? 1 : i + 1))
		};
		indices.insert(indices.end(), fanIndices.begin(), fanIndices.end());
	}

	return indices;
}

bool lessPoint(Point a, Point b, Point center)
{
	if (a.x - center.x >= 0 && b.x - center.x < 0)
		return true;
	else if (a.x - center.x < 0 && b.x - center.x >= 0)
		return false;
	else if (a.x - center.x == 0 && b.x - center.x == 0)
	{
		if (a.z - center.z >= 0 || b.z - center.z >= 0)
			return a.z > b.z;
		else
			return b.z > a.z;
	}

	auto det = (a.x - center.x) * (b.z - center.z) - (b.x - center.x) * (a.z - center.z);
	if (det < 0)
		return true;
	else if (det > 0)
		return false;

	auto detA = (a.x - center.x) * (a.x - center.x) + (a.z - center.z) * (a.z - center.z);
	auto detB = (b.x - center.x) * (b.x - center.x) + (b.z - center.z) * (b.z - center.z);
	return detA > detB;
}

Points getCircleSortedPoints(Points points, bool clockwise)
{
	auto cursor = points.begin();
	while (cursor != points.end())
	{
		auto comparator = cursor + 1;
		if (comparator == points.end())
			break;

		auto smallest = comparator++;
		while (comparator != points.end())
		{
			if (glm::length(*cursor - *comparator) < glm::length(*cursor - *smallest))
				std::swap(*smallest, *comparator++);
		}

		++cursor;
	}

	glm::vec3 avg{};
	for (const auto& point : points)
		avg += point;
	avg /= float(points.size());

	bool isClockwise = lessPoint(points[0], points[1], avg);

	if (isClockwise && !clockwise || !isClockwise && clockwise)
		std::reverse(std::begin(points), std::end(points));

	return points;
}

void RoadIntersection::construct(std::array<Road*, 3> roads, Point intersectionPoint)
{
	m_position = intersectionPoint;
	centre = glm::vec3(0);
	width = roads[0]->getParameters().width;


	{
		float requiredDistance = width / 2.0f + 0.25;
		for (auto& road : roads)
		{
			auto parameters = road->getParameters();
			connectionPoints.push_back(road->shorten(intersectionPoint, requiredDistance) - m_position);
		}
	}

	Points sidePoints;
	Points commonPoints;
	for (int i = 0; i < roads.size(); ++i)
	{
		auto cps = getCircleSortedPoints(connectionPoints, false);
		//cps.size() == 3
		// find neigbbour axes
		auto first = cps[(i + 1) % cps.size()];
		auto second = cps[(i + 2) % cps.size()];

		auto& currentPoint = cps[i];
		glm::vec3 dirCurrentToCentre = glm::normalize(centre - currentPoint);
		glm::vec3 dirCentreToFirst = glm::normalize(first - centre);
		glm::vec3 dirCentreToSecond = glm::normalize(second - centre);

		{
			const auto [left, right] = getSidePoints(dirCurrentToCentre, dirCurrentToCentre, currentPoint, currentPoint, centre, width);

			sidePoints.insert(sidePoints.end(), { left, right });
		}
		
		{
			//anti clockwise so left point we take
			const auto [ _, right] = getSidePoints(dirCurrentToCentre, dirCentreToFirst, currentPoint, centre, first, width);
			//	const auto [left,right] = getSidePoints(dirCurrentToCentre, dirCentreToSecond, currentPoint, centre, second, width);
			commonPoints.insert(commonPoints.end(), { right });

			// just for fun
		}
	}
	// + centre
	size_t totalPoints = sidePoints.size() + commonPoints.size() + 1;
	VD::PositionVertices shapePoints(totalPoints);
	auto pointsIter = shapePoints.begin();
	*pointsIter++ = centre;
	for (int sideIndex = 1, commonIndex = 0; sideIndex < sidePoints.size(); sideIndex +=2, ++commonIndex)
	{
		// side, side, common
		*(pointsIter) = sidePoints[sideIndex - 1];
		*(pointsIter + 1) = sidePoints[sideIndex];

		// in case there are odd common points
		*(pointsIter + 2) = commonPoints[commonIndex];

		// vertices added
		std::advance(pointsIter, 3);
	}
	
	VD::Indices indices = createPseudoTriangleFanIndices(shapePoints);
	Mesh mesh;
	mesh.vertices.positions = shapePoints;

	const glm::vec4 grey(0.44, 0.44, 0.44, 1.0);
	VD::ColorVertices colors(shapePoints.size(), grey);
	mesh.vertices.colors = colors;
	mesh.indices = indices;

	Model model;
	model.meshes.push_back(mesh);

	Info::ModelInfo mInfo;
	mInfo.model = &model;

	setupModel(mInfo, true);
}
