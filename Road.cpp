#include "Road.h"
#include "Utilities.h"
#include "Mesh.h"

#include <glm/gtx/string_cast.hpp>
#include <numeric>
#include <random>
#include <chrono>
#include <array>


float calculateLength(const Points& points)
{
	float length = 0;

	auto pointIterator = points.begin();
	while (pointIterator != points.end() - 1)
	{
		length += glm::length(*pointIterator - *(pointIterator + 1));
		++pointIterator;
	}

	return length;
}

glm::vec3 getAveragePosition(const Points& points)
{
	glm::vec3 averagePosition = std::accumulate(std::begin(points), std::end(points), glm::vec3());

	return averagePosition / float(points.size());
}

std::tuple<Points::iterator, Points::iterator, float> travellDistanceOnPoints(Points& points, float distance)
{
	float travelledDistance = 0;
	auto currentPoint = points.begin();
	auto nextPoint = currentPoint + 1;
	while (nextPoint != points.end())
	{
		travelledDistance += glm::length(*currentPoint - *nextPoint);

		if (travelledDistance >= distance)
			break;
		else
			currentPoint = nextPoint++;
	}

	return std::make_tuple(currentPoint, nextPoint, travelledDistance);
}

bool polygonPointCollision(const Points& polygon, const Point& point)
{
	bool collision = false;
	glm::vec3 hitPos;
	for (auto vert = polygon.begin(); vert != polygon.end(); ++vert)
	{
		auto nextVert = (vert + 1) ==
			polygon.end() ? polygon.begin() : vert + 1;

		// z test
		if ( (vert->z >= point.z && nextVert->z < point.z) || (vert->z < point.z && nextVert->z >= point.z))
		{
			if (point.x < (nextVert->x - vert->x) * (point.z - vert->z) / (nextVert->z - vert->z) + vert->x)
			{
				collision = !collision;
				if (collision)
					hitPos = point;
			}
		}
	}

	return collision;
}

bool polygonPointCollision(const Points& vertices, float px, float py)
{
	bool collision = false;

	// go through each of the vertices, plus
	// the next vertex in the list
	int next = 0;
	for (int current = 0; current < vertices.size(); current++) {

		// get next vertex in list
		// if we've hit the end, wrap around to 0
		next = current + 1;
		if (next == vertices.size()) next = 0;

		// get the PVectors at our current position
		// this makes our if statement a little cleaner
		Point vc = vertices[current];    // c for "current"
		Point vn = vertices[next];       // n for "next"

		// compare position, flip 'collision' variable
		// back and forth
		if (((vc.z >= py && vn.z < py) || (vc.z < py && vn.z >= py)) &&
			(px < (vn.x - vc.x) * (py - vc.z) / (vn.z - vc.z) + vc.x)) 
		{
			collision = !collision;
		}
	}
	return collision;
}

bool polygonPolygonCollision(const Points& polygonOne, const Points& polygonTwo)
{
	bool collided = false;
	for (const auto& point : polygonOne)
	{
		if (polygonPointCollision(polygonTwo, point))
		{
			collided = true;
			break;
		}
	}

	return collided;
}

Points createShape(const Points& points, float width)
{
	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	Points leftPoints;
	Points rightPoints;

	glm::vec3 dirVec;
	glm::vec3 currentDirection;
	glm::vec3 previousDirection;
	Point previousPoint;
	Point nextPoint;
	for (int i = 0; i < points.size(); ++i)
	{
		// vertices
		{
			const auto& curPoint = points[i];
			previousPoint = curPoint;
			if (i + 1 < points.size())
			{
				nextPoint = points[i + 1];
				currentDirection = glm::normalize(nextPoint - curPoint);
			}
			else if (points.front() == points.back())
			{
				nextPoint = *(points.begin() + 1);
				currentDirection = glm::normalize(nextPoint - curPoint);
			}

			if (i - 1 >= 0)
			{
				previousPoint = points[i - 1];
				previousDirection = glm::normalize(curPoint - previousPoint);
			}
			else if (points.front() == points.back())
			{
				previousPoint = *(points.end() - 2);
				previousDirection = glm::normalize(curPoint - previousPoint);
			}
			else
			{
				previousDirection = currentDirection;
			}

			const auto [left, right] = getSidePoints(previousDirection, currentDirection, previousPoint, curPoint, nextPoint, width);

			leftPoints.emplace_back(left);
			rightPoints.emplace_back(right);
		}
	}
	Points shapePoints;
	shapePoints.insert(std::end(shapePoints), std::begin(leftPoints), std::end(leftPoints));
	shapePoints.insert(std::end(shapePoints), std::rbegin(rightPoints), std::rend(rightPoints));

	return shapePoints;
}

Mesh creteTexturedMesh(const Points& points, int width)
{
	VD::PositionVertices vertices;
	VD::TextureVertices textureCoords;
	VD::Indices indices;

	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	float shapeLength = calculateLength(points);
	float textureDistance = 0;

	glm::vec3 dirVec;
	glm::vec3 currentDirection;
	glm::vec3 previousDirection;
	Point previousPoint;
	Point nextPoint;
	for (int i = 0; i < points.size(); ++i)
	{
		// vertices
		{
			const auto& curPoint = points[i];
			previousPoint = curPoint;
			if (i + 1 < points.size())
			{
				nextPoint = points[i + 1];
				currentDirection = glm::normalize(nextPoint - curPoint);
			}
			else if (points.front() == points.back())
			{
				nextPoint = *(points.begin() + 1);
				currentDirection = glm::normalize(nextPoint - curPoint);
			}

			if (i - 1 >= 0)
			{
				previousPoint = points[i - 1];
				previousDirection = glm::normalize(curPoint - previousPoint);
			}
			else if (points.front() == points.back())
			{
				previousPoint = *(points.end() - 2);
				previousDirection = glm::normalize(curPoint - previousPoint);
			}
			else
			{
				previousDirection = currentDirection;
			}

			const auto [left, right] = getSidePoints(previousDirection, currentDirection, previousPoint, curPoint, nextPoint, width);

			std::array<VD::PositionVertex, 2> sideVertices;
			sideVertices[0] = right;
			sideVertices[1] = left;

			vertices.insert(vertices.end(), { left, right });
		}
		// textures
		{
			if (i != 0)
				textureDistance += glm::length(points[i - 1] - points[i]);

			VD::TextureVertex rightCoord = glm::vec2(1, textureDistance);
			VD::TextureVertex leftCoord = glm::vec2(0, textureDistance);

			textureCoords.insert(textureCoords.end(), { leftCoord, rightCoord });
		}
	}

	// indices
	for (int i = 0; i < vertices.size(); ++i)
	{
		if (i > 1)
		{
			std::array<uint32_t, 3> triplets = { i - 2, i - 1, i };
			indices.insert(std::end(indices), std::begin(triplets), std::end(triplets));
		}
	}

	Mesh mesh;
	mesh.vertices.positions = vertices;
	mesh.vertices.textures = textureCoords;
	mesh.indices = indices;

	return mesh;
}

std::vector<Point> createOutline(const std::vector<Point>& points, float outlineSize)
{
	if (points.size() < 2)
		return points;

	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	std::vector<Point> leftPoints;
	std::vector<Point> rightPoints;

	glm::vec3 dirVec;
	glm::vec3 currentDirection;
	glm::vec3 previousDirection;
	Point previousPoint;
	Point nextPoint;
	for (int i = 0; i < points.size(); ++i)
	{
		const auto& curPoint = points[i];
		previousPoint = curPoint;
		if (i + 1 < points.size())
		{
			nextPoint = points[i + 1];
			currentDirection = glm::normalize(nextPoint - curPoint);
		}
		else if (points.front() == points.back())
		{
			nextPoint = *(points.begin() + 1);
			currentDirection = glm::normalize(nextPoint - curPoint);
		}

		if (i - 1 >= 0)
		{
			previousPoint = points[i - 1];
			previousDirection = glm::normalize(curPoint - previousPoint);
		}
		else if (points.front() == points.back())
		{
			previousPoint = *(points.end() - 2);
			previousDirection = glm::normalize(curPoint - previousPoint);
		}
		else
		{
			previousDirection = currentDirection;
		}

		const auto [left, right] = getSidePoints(previousDirection, currentDirection, previousPoint, curPoint, nextPoint, outlineSize);
			// dont duplicate first and last
		if (i == 0 || i == points.size() - 1)
		{
			leftPoints.emplace_back(left);
			rightPoints.emplace_back(right);
		}
		else
		{
			leftPoints.insert(leftPoints.end(), { left, left });
			rightPoints.insert(rightPoints.end(), { right, right });
		}
	}
	std::vector<Point> shapePoints(leftPoints.size() + rightPoints.size());
	auto insertIt = std::copy(std::begin(leftPoints), std::end(leftPoints), shapePoints.begin());
	insertIt = std::copy(std::begin(rightPoints), std::end(rightPoints), insertIt);

	std::vector<Point> outline(shapePoints.size());
	auto outIt = outline.begin();
	for (auto& point : shapePoints)
		*outIt++ = point * outlineSize;
	
	return outline;

}

bool approxSamePoints(const Point& p1, const Point& p2)
{
	constexpr const float maxDiff = 0.01f;

	return glm::length(p1 - p2) <= maxDiff;
}

bool arePointsInRange(const Point& p1, const Point& p2, float range)
{
	return glm::length(p1 - p2) <= range;
}

Road::RoadParameters Road::getParameters() const
{
	return m_parameters;
}

Point Road::getHead() const
{
	return m_parameters.axis.world.back();
}

Point Road::getTail() const
{
	return m_parameters.axis.world.front();
}

bool Road::sitsOnHead(const Point& point)
{
	return approxSamePoints(point, getHead());
}

bool Road::sitsOnTail(const Point& point)
{
	return approxSamePoints(point, getTail());
}

bool Road::sitsOnTailOrHead(const Point& point)
{
	return sitsOnTail(point) || sitsOnHead(point);
}

bool Road::sitsOnRoad(const Point& point)
{
	auto endPoint = point - m_position;
	return polygonPointCollision(m_modelShape, endPoint);
}

void Road::reverseBody()
{
	std::reverse(std::begin(m_parameters.axis.local), std::end(m_parameters.axis.local));
	std::reverse(std::begin(m_parameters.axis.world), std::end(m_parameters.axis.world));
}

Point Road::getPointOnRoad(const Point& point)
{
	const auto [pointOne, pointTwo] = findTwoClosestPoints(m_parameters.axis.world, point);
	glm::vec3 directionPoint = pointTwo - pointOne;
	float lineLength = glm::length(directionPoint);

	const glm::vec3 dirTwoToOne = glm::normalize(pointTwo - pointOne);
	const glm::vec3 dirOneToPoint = glm::normalize(point - pointOne);
	float alpha = glm::acos(glm::dot(dirOneToPoint, dirTwoToOne));

	float lineDistanceFromPointOne = glm::length(pointOne - point) * cos(alpha);

	float percentageDistance = lineDistanceFromPointOne / lineLength;
	Point roadPoint = pointOne + directionPoint * percentageDistance;

	const float minDistanceFromEndPoints = m_parameters.width;
	if (auto head = getHead(), tail = getTail(); arePointsInRange(head, roadPoint, minDistanceFromEndPoints) ||
		arePointsInRange(tail, roadPoint, minDistanceFromEndPoints))
		return glm::length(head - point) < glm::length(tail - point) ? head : tail;
	else
		return roadPoint;
}

void Road::construct(Points axisPoints, uint32_t laneCount, float width, std::string texture)
{
	m_position = getAveragePosition(axisPoints);

	m_parameters = {};
	m_parameters.axis.world = axisPoints;
	std::transform(std::begin(axisPoints), std::end(axisPoints), std::back_inserter(m_parameters.axis.local),
		[&](const Point& point) { return point - m_position; });

	m_parameters.laneCount = laneCount;
	m_parameters.width = width;
	m_parameters.texture = texture;

	m_modelShape = createShape(m_parameters.axis.local, m_parameters.width);
	// model
	auto mesh = creteTexturedMesh(m_parameters.axis.local, m_parameters.width);
	mesh.textures[VD::TextureType::DIFFUSE] = texture;

	Model model;
	model.meshes.push_back(mesh);

	Info::ModelInfo modelInfo;
	modelInfo.model = &model;

	setupModel(modelInfo, true);
	createPaths();
}

void Road::mergeWithRoad(Road& road)
{
	// sits on opposite sides
	if (approxSamePoints(getHead(), road.getHead()) || approxSamePoints(getTail(), road.getTail()))
		road.reverseBody();
	// copy the right direction
	if (approxSamePoints(getTail(), road.getHead()))
	{
		m_parameters.axis.world.insert(std::begin(m_parameters.axis.world), 
			std::begin(road.m_parameters.axis.world), std::end(road.m_parameters.axis.world) - 1);
		construct(m_parameters.axis.world, m_parameters.laneCount, m_parameters.width, m_parameters.texture);
	}
	else
	{
		std::copy(std::begin(road.m_parameters.axis.world) + 1, std::end(road.m_parameters.axis.world), std::back_inserter(m_parameters.axis.world));
		construct(m_parameters.axis.world, m_parameters.laneCount, m_parameters.width, m_parameters.texture);
	}
}

std::optional<Road> Road::split(const Point& splitPoint)
{
	std::optional<Road> optionalSplit;

	if (!sitsOnTailOrHead(splitPoint))
	{
		const auto [firstPoint, secondPoint] = findTwoClosestPoints(m_parameters.axis.world, splitPoint);

		Points newAxis = m_parameters.axis.world;
		auto newSplitIter = insertElemementBetween(newAxis, firstPoint, secondPoint, splitPoint);
		// erase if same point
		if (approxSamePoints(*newSplitIter, firstPoint) || approxSamePoints(*newSplitIter, secondPoint))
			newSplitIter = newAxis.erase(newSplitIter);

		// recteate this worldAxis
		auto& curWorldAxis = m_parameters.axis.world;
		curWorldAxis.erase(std::copy(std::begin(newAxis), newSplitIter + 1, std::begin(curWorldAxis)), std::end(curWorldAxis));

		Points secondSplitAxis(newSplitIter, std::end(newAxis));


		construct(curWorldAxis, m_parameters.laneCount, m_parameters.width, m_parameters.texture);

		Road optRoad;
		optRoad.construct(secondSplitAxis, m_parameters.laneCount, m_parameters.width, m_parameters.texture);
		optionalSplit = optRoad;
	}

	return optionalSplit;
}

Point Road::shorten(const Point& roadEnd, float size)
{
	Point shortenPosition = {};
	if (sitsOnTail(roadEnd))
	{
		auto [firstPoint, secondPoint, travelledDistance] = travellDistanceOnPoints(m_parameters.axis.world, size);
		if (secondPoint == std::end(m_parameters.axis.world))
			throw std::runtime_error("Too far size to shorten");

		float newVecLength = travelledDistance - size;
		if (newVecLength != 0)
		{
			// reverse direction and go from second point
			glm::vec3 direction = glm::normalize(*firstPoint - *secondPoint);
			Point pointPos = *secondPoint + (direction * newVecLength);

			// overwrite
			shortenPosition = *firstPoint = pointPos;
		}

		// remove unsuitable points
		m_parameters.axis.world.erase(m_parameters.axis.world.begin(), firstPoint);
	}
	else
	{
		//reversed 
		Points reversedAxis;
		std::reverse_copy(std::begin(m_parameters.axis.world), std::end(m_parameters.axis.world), std::back_inserter(reversedAxis));
		auto [firstPoint, secondPoint, travelledDistance] = travellDistanceOnPoints(reversedAxis, size);

		if (firstPoint == std::end(reversedAxis))
			throw std::runtime_error("Too far size to shorten");

		float newVecLength = travelledDistance - size;
		if (newVecLength != 0)
		{
			// reverse direction and go from second point
			glm::vec3 direction = glm::normalize(*firstPoint - *secondPoint);
			Point pointPos = *secondPoint + (direction * newVecLength);

			// overwrite
			shortenPosition = *firstPoint = pointPos;
		}

		// remove unsuitable points
		auto copyIt = std::reverse_copy(firstPoint, std::end(reversedAxis), std::begin(m_parameters.axis.world));
		m_parameters.axis.world.erase(copyIt, std::end(m_parameters.axis.world));
	}

	construct(m_parameters.axis.world, m_parameters.laneCount, m_parameters.width, m_parameters.texture);
	return shortenPosition;
}

void Road::createPaths()
{
	float offsetPerLane = (m_parameters.laneCount / m_parameters.width) / 2.0f;
	float startOffset = 0.25f;

	// supposse we have two lanes
	m_paths.clear();
	for (int i = 0; i < m_parameters.laneCount / 2.0; ++i)
	{
		auto pathLanes = createShape(m_parameters.axis.local, startOffset + offsetPerLane * i);

		m_paths.insert(m_paths.begin(), std::vector(pathLanes.begin() + (pathLanes.size() / 2), pathLanes.end()));
		m_paths.insert(m_paths.end(), std::vector(pathLanes.begin(), pathLanes.begin() + (pathLanes.size() / 2)));
	}
}
