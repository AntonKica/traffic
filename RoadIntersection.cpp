#include "RoadIntersection.h"
#include "Utilities.h"
#include "Road.h"

#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


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
				std::swap(*smallest, *comparator);
			++comparator;
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
	m_position = {};
	m_centre = intersectionPoint;
	m_width = roads[0]->getParameters().width;
	m_connectionPoints.clear();

	{
		float requiredDistance = m_width / 2.0f + 0.25;
		for (auto& road : roads)
		{
			auto parameters = road->getParameters();
			auto shortPoint = road->shorten(intersectionPoint, requiredDistance);
			m_connectionPoints.push_back(shortPoint);

			connect(road, shortPoint);
			road->reconstruct();
		}
	}

	Points sidePoints;
	Points commonPoints;
	for (int i = 0; i < roads.size(); ++i)
	{
		auto cps = getCircleSortedPoints(m_connectionPoints, false);
		//cps.size() == 3
		// find neigbbour axes
		auto first = cps[(i + 1) % cps.size()];
		auto second = cps[(i + 2) % cps.size()];

		auto& currentPoint = cps[i];
		glm::vec3 dirCurrentToCentre = glm::normalize(m_centre - currentPoint);
		glm::vec3 dirCentreToFirst = glm::normalize(first - m_centre);
		glm::vec3 dirCentreToSecond = glm::normalize(second - m_centre);

		{
			auto connectP = findConnection(currentPoint).connected->getDirectionPointFromConnectionPoint(currentPoint);
			glm::vec3 dirConnectToCurrent = glm::normalize(currentPoint - connectP);

			const auto [left, right] = getSidePoints(dirConnectToCurrent, dirCurrentToCentre, connectP, currentPoint, m_centre, m_width);

			sidePoints.insert(sidePoints.end(), { left, right });
		}
		
		{
			//anti clockwise so left point we take
			const auto [ _, right] = getSidePoints(dirCurrentToCentre, dirCentreToFirst, currentPoint, m_centre, first, m_width);
			//	const auto [left,right] = getSidePoints(dirCurrentToCentre, dirCentreToSecond, currentPoint, centre, second, width);
			commonPoints.insert(commonPoints.end(), { right });

			// just for fun
		}
	}
	// + centre
	size_t totalPoints = sidePoints.size() + commonPoints.size() + 1;
	VD::PositionVertices shapePoints(totalPoints);
	auto pointsIter = shapePoints.begin();
	*pointsIter++ = m_centre;
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

glm::vec3 RoadIntersection::getDirectionPointFromConnectionPoint(Point connectionPoint)
{
	return m_centre;
}

BasicRoad::ConnectionPossibility RoadIntersection::canConnect(Line connectionLine, Point connectionPoint) const
{
	ConnectionPossibility cp{};
	return cp;
}
