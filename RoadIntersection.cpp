#include "RoadIntersection.h"
#include "Utilities.h"
#include "Collisions.h"
#include "Road.h"

#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


VD::Indices createPseudoTriangleFanIndices(const Points& points)
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

Points createTriangleFanPointsFromPointsAndFanPoint(const Points& points, Point fanPoint)
{
	const uint32_t pointsCount = (points.size() + 1) * 3;
	Points triangleFanPoints(pointsCount);
	auto dataIter = triangleFanPoints.begin();

	for (auto pIter = points.begin(), pIterNext = points.begin() + 1; pIter != points.end(); ++pIter, ++pIterNext)
	{
		if (pIterNext == points.end())
			pIterNext = points.begin();

		*(dataIter++) = fanPoint;
		*(dataIter++) = *pIter;
		*(dataIter++) = *pIterNext;
	}

	return triangleFanPoints;
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

	// adjust connectd roads
	{
		float requiredDistance = m_width / 2.0f + 0.25;
		for (auto& road : roads)
		{
			auto parameters = road->getParameters();
			auto shortPoint = road->shorten(Shape::AxisPoint(intersectionPoint), requiredDistance);
			m_connectionPoints.push_back(shortPoint);

			connect(road, shortPoint);
			road->reconstruct();
		}
	}

	setUpShape();
}

void RoadIntersection::setUpShape()
{
	Points sidePoints;
	Points commonPoints;

	// sort them for easier manipulation
	auto circleSortedPoints = getCircleSortedPoints(m_connectionPoints, false);
	for (int i = 0; i < circleSortedPoints.size(); ++i)
	{
		auto cps = getCircleSortedPoints(m_connectionPoints, false);
		//cps.size() == 3
		// find neigbbour axes
		auto first = circleSortedPoints[(i + 1) % circleSortedPoints.size()];
		auto second = circleSortedPoints[(i + 2) % circleSortedPoints.size()];

		auto& currentPoint = circleSortedPoints[i];
		glm::vec3 dirCurrentToCentre = glm::normalize(m_centre - currentPoint);
		glm::vec3 dirCentreToFirst = glm::normalize(first - m_centre);
		glm::vec3 dirCentreToSecond = glm::normalize(second - m_centre);

		{
			// get point from connected road
			auto connectionPoint = getConnection(currentPoint).connected->getDirectionPointFromConnectionPoint(currentPoint);
			glm::vec3 dirConnectToCurrent = glm::normalize(currentPoint - connectionPoint);

			const auto [left, right] = getSidePoints(dirConnectToCurrent, dirCurrentToCentre, connectionPoint, currentPoint, m_centre, m_width);

			sidePoints.insert(sidePoints.end(), { left, right });
		}

		{
			const auto [_, right] = getSidePoints(dirCurrentToCentre, dirCentreToFirst, currentPoint, m_centre, first, m_width);
			commonPoints.insert(commonPoints.end(), { right });
		}
	}

	// merge points 
	const size_t totalPoints = sidePoints.size() + commonPoints.size();
	m_outlinePoints.resize(totalPoints);
	auto pointsIter = m_outlinePoints.begin();
	for (int sideIndex = 1, commonIndex = 0; sideIndex < sidePoints.size(); sideIndex += 2, ++commonIndex)
	{
		// side, side, common
		*(pointsIter) = sidePoints[sideIndex - 1];
		*(pointsIter + 1) = sidePoints[sideIndex];

		// in case there are odd common points
		*(pointsIter + 2) = commonPoints[commonIndex];

		// vertices added
		std::advance(pointsIter, 3);
	}

	m_shapePoints = createTriangleFanPointsFromPointsAndFanPoint(m_outlinePoints, m_centre);

	// then setupDrawing
	Mesh mesh;
	mesh.vertices.positions = m_shapePoints;

	const glm::vec4 grey(0.44, 0.44, 0.44, 1.0);
	VD::ColorVertices colors(m_shapePoints.size(), grey);
	mesh.vertices.colors = colors;

	Model model;
	model.meshes.push_back(mesh);

	Info::ModelInfo mInfo;
	mInfo.model = &model;

	setupModel(mInfo, true);
}

bool RoadIntersection::validIntersection()
{
	return m_connections.size() >= 3;
}

std::vector<Road*> RoadIntersection::disassemble()
{
	//set cause there may be two same roads
	std::set<Road*> connected;
	if (m_connections.size())
	{
		auto curConnection = m_connections[0];
		dismissConnection(curConnection);

		auto road = dynamic_cast<Road*>(curConnection.connected);


		road->extend(Shape::AxisPoint(curConnection.point), m_centre);
		connected.insert(road);

		if (m_connections.size())
		{
			curConnection = m_connections[0];
			dismissConnection(curConnection);

			road->extend(Shape::AxisPoint(m_centre), curConnection.point);

			connected.insert(dynamic_cast<Road*>(curConnection.connected));

		}
	}

	return std::vector(std::begin(connected), std::end(connected));
}

glm::vec3 RoadIntersection::getDirectionPointFromConnectionPoint(Point connectionPoint)
{
	return m_centre;
}

BasicRoad::ConnectionPossibility RoadIntersection::getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const
{
	ConnectionPossibility cp{};
	cp.recomendedPoint = connectionPoint;
	return cp;
}

void RoadIntersection::destroy()
{
}

bool RoadIntersection::hasBody() const
{
	return m_connectionPoints.size();
}

bool RoadIntersection::sitsPointOn(Point point) const
{
	return Collisions::polygonPointCollision(m_outlinePoints, point);
}

BasicRoad::RoadType RoadIntersection::getRoadType() const
{
	return RoadType::INTERSECTION;
}

Shape::AxisPoint RoadIntersection::getAxisPoint(Point pointOnRoad) const
{
	return Shape::AxisPoint(m_centre);
}

