#include "Road.h"
#include "DataManager.h"
#include "Utilities.h"

#include <glm/gtx/string_cast.hpp>
#include <numeric>
#include <random>
#include <chrono>


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

glm::vec3 getAveratePosition(const Points& points)
{
	glm::vec3 averagePosition = std::accumulate(std::begin(points), std::end(points), glm::vec3());

	return averagePosition / float(points.size());
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

std::pair<GO::TypedVertices, GO::Indices> createTexturedShape(const Points& points, int width)
{
	GO::Indices indices;
	GO::TypedVertices typedVts;
	auto& [type, vertices] = typedVts;
	type = GO::VertexType::TEXTURED;

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
		std::array<GO::VariantVertex, 2> variantVertex;
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

			variantVertex[0].texturedVertex.position = right;
			variantVertex[1].texturedVertex.position = left;
		}
		// textures
		{
			if (i != 0)
				textureDistance += glm::length(points[i - 1] - points[i]);

			glm::vec2 rightCoord = glm::vec2(1, textureDistance);
			glm::vec2 leftCoord = glm::vec2(0, textureDistance);

			variantVertex[0].texturedVertex.texCoord = leftCoord;
			variantVertex[1].texturedVertex.texCoord = rightCoord;
		}
		vertices.insert(vertices.end(), { variantVertex[0], variantVertex[1] });
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

	return std::make_pair(typedVts, indices);
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


 std::optional<Road> Road::splitRoad(const Point& splitPoint)
{
	std::optional<Road> optionalSplit;

	if (!glm::length(splitPoint - parameters.worldAxis.front()) <= 0.01 && !glm::length(splitPoint - parameters.worldAxis.back()) <= 0.01)
	{
		const auto [firstPoint, secondPoint] = findTwoClosestPoints(parameters.worldAxis, splitPoint);

		Points newAxis = parameters.worldAxis;
		int splitIndex = insertElemementBetween(newAxis, firstPoint, secondPoint, splitPoint);

		auto positionTransform = [&](const Point& point) { return point + m_position; };
		// recteate this worldAxis
		parameters.worldAxis.clear();
		std::copy(std::begin(newAxis), std::begin(newAxis) + splitIndex + 1, std::back_inserter(parameters.worldAxis));
		Points secondSplitAxis(std::begin(newAxis) + splitIndex, std::end(newAxis));


		construct(parameters.worldAxis, parameters.laneCount, parameters.width, parameters.texture);

		Road optRoad;
		optRoad.construct(secondSplitAxis, parameters.laneCount, parameters.width, parameters.texture);
		optionalSplit = optRoad;
	}

	return optionalSplit;
}

Point Road::shorten(Point endPoint, float cutSize)
{
	Point shortedPoint = {};
	if (endPoint == parameters.worldAxis.front())
	{
		// find moste sutablePoint
		auto firstPoint = parameters.worldAxis.begin();
		auto secondPoint = firstPoint + 1;
		int count = 0;
		while (secondPoint != parameters.worldAxis.end())
		{
			float distance = glm::length(endPoint - *secondPoint);
			if (cutSize > distance)
				firstPoint = secondPoint++;
			else //found
				break;
			++count;
		}

		float vecMagnutude = glm::length(*secondPoint - endPoint) - cutSize;
		if (vecMagnutude != 0)
		{
			// reverse direction and go from second point
			glm::vec3 direction = glm::normalize(*firstPoint - *secondPoint);
			Point pointPos = *secondPoint + (direction * vecMagnutude);

			// overwrite
			*firstPoint = pointPos;
		}

		// remove unsuitable points
		parameters.worldAxis.erase(parameters.worldAxis.begin(), firstPoint);
		shortedPoint = parameters.worldAxis.front();
	}
	else
	{
		auto firstPoint = parameters.worldAxis.rbegin();
		auto secondPoint = firstPoint + 1;
		while (secondPoint != parameters.worldAxis.rend())
		{
			float distance = glm::length(endPoint - *secondPoint);
			if (cutSize > distance)
				firstPoint = secondPoint++;
			else //found
				break;
		}

		float vecMagnutude = glm::length(*secondPoint - *firstPoint) - cutSize;
		if (vecMagnutude != 0)
		{
			glm::vec3 direction = glm::normalize(*secondPoint - *firstPoint);
			Point pointPos = *firstPoint + (direction * vecMagnutude);

			// overwrite
			*firstPoint = pointPos;
		}

		// change to normal direction
		auto normIter = firstPoint.base();
		//std::advance(normIter, -1);
		// remove unsuitable points
		parameters.worldAxis.erase(normIter, parameters.worldAxis.end());
		shortedPoint = parameters.worldAxis.back();
	}
	//glm::vec3 shiftPosition = getAveratePosition(parameters.worldAxis);

	construct(parameters.worldAxis, parameters.laneCount, parameters.width, parameters.texture);
	return shortedPoint;
}


std::vector<Point> generateCurveFromThreePoints(const std::array<Point, 3>& points, int precision)
{
	// on same line
	if (points[0] == points[1] && points[1] == points[2])
		return std::vector(points.begin(), points.end());

	std::vector<Point> curvePoints(precision + 1);

	const glm::vec3 firstDir = glm::normalize(points[1] - points[0]);
	const float firstLength = glm::length(points[1] - points[0]);
	const glm::vec3 secondtDir = glm::normalize(points[2] - points[1]);
	const float secondtLenght = glm::length(points[2] - points[1]);

	for (int i = 0; i <= precision; ++i)
	{
		float distanceInPercentage = i / float(precision);

		const float firstMagnitude = 1.0 - distanceInPercentage;
		const float firstDistance = std::lerp(0, firstLength, distanceInPercentage);
		const glm::vec3 firstPoint = (points[0] + firstDir * firstDistance) * firstMagnitude;
		const float secondMagnitude = distanceInPercentage;
		const float secondDistance = std::lerp(0, secondtLenght, distanceInPercentage);
		const glm::vec3 secondPoint = (points[1] + secondtDir * secondDistance) * secondMagnitude;

		const glm::vec3 curvePoint = firstPoint + secondPoint;
		curvePoints[i] = curvePoint;
	}

	return curvePoints;
}
Point findEqalPoint(const Points& points1, const Points& points2)
{
	for (const Point& p1 : points1)
	{
		for (const Point& p2 : points2)
		{
			if (p1 == p2)
			{
				return p1;
				break;
			}
		}
	}
	throw std::runtime_error("No equal elements");
}
Road Road::mergeRoads(std::pair<Road, Road> roads)
{
	auto& [firstRoad, secondRoad] = roads;
	auto& worldAxisOne = firstRoad.parameters.worldAxis;
	auto& worldAxisTwo = secondRoad.parameters.worldAxis;

	//std::set_intersection(std::begin(worldAxisOne), std::end(worldAxisOne), std::begin(worldAxisTwo), std::end(worldAxisTwo))
	Point intersection;
	int intersectionCount = 0;
	for (const Point& p1 : worldAxisOne)
	{
		for (const Point& p2 : worldAxisTwo)
		{
			// compare by length since inaccuate results
			if (glm::length((p1) - (p2)) <= glm::epsilon<float>())
			{
				if (intersectionCount == 0)
					intersection = p1;
				++intersectionCount;
				break;
			}
		}
	}

	if (intersection != worldAxisOne.back())
	{
		std::reverse(std::begin(worldAxisOne), std::end(worldAxisOne));
	}

	if (intersection + firstRoad.m_position != worldAxisTwo.front() + secondRoad.m_position)
	{
		std::reverse(std::begin(worldAxisTwo), std::end(worldAxisTwo));
	}

	//glm::vec3 averagePos = (m_position - road.m_position) / 2.0f;

//	Points curvePoints = generateCurveFromThreePoints({ worldAxisOne[worldAxisOne.size() - 2] + firstRoad.m_position, worldAxisOne[worldAxisOne.size() - 1] + firstRoad.m_position, worldAxisTwo[1] + secondRoad.m_position }, 3);
	Points concatedAxis(std::begin(worldAxisOne), std::end(worldAxisOne));
	std::copy(std::begin(worldAxisTwo) + 1, std::end(worldAxisTwo), std::back_inserter(concatedAxis));

	Road concatedRoad;
	concatedRoad.construct(concatedAxis, firstRoad.parameters.laneCount, firstRoad.parameters.width, firstRoad.parameters.texture);

	return concatedRoad;
}

void Road::createPaths()
{
	auto& lanes = parameters.lanes;

	float offsetPerLane = (parameters.laneCount / parameters.width) / 2.0f;
	float startOffset = 0.25f;

	// supposse we have two lanes
	lanes.clear();
	for (int i = 0; i < parameters.laneCount / 2.0; ++i)
	{
		auto pathLanes = createShape(parameters.localAxis, startOffset + offsetPerLane * i);

		lanes.insert(lanes.begin(), std::vector(pathLanes.begin() + (pathLanes.size() / 2), pathLanes.end()));
		lanes.insert(lanes.end(), std::vector(pathLanes.begin(), pathLanes.begin() + (pathLanes.size() / 2)));
	}
}

Points Road::createLocalPoints(const Points& points, const glm::vec3& position)
{
	Points endPoints(points.size());
	std::transform(std::begin(points), std::end(points), std::begin(endPoints),
		[&position](const Point& point)
		{
			return point - position;
		});

	return endPoints;
}


void Road::construct(const Points& points, uint32_t laneCount, float width, std::string texture)
{
	auto position = getAveratePosition(points);
	auto endPoints = createLocalPoints(points, position);

	construct(endPoints, position, laneCount, width, texture);
}

void Road::construct(const Points& localAxis, glm::vec3 position, uint32_t laneCount, float width, std::string texture)
{
	m_position = position;

	parameters.worldAxis.clear();
	std::transform(begin(localAxis), end(localAxis), std::back_inserter(parameters.worldAxis), [&position](const Point& point) {return point + position; });

	parameters.localAxis = localAxis;
	parameters.model = createShape(localAxis, width);
	parameters.laneCount = laneCount;
	parameters.width = width;
	parameters.texture = texture;

	const auto [typedVertices, indices] = createTexturedShape(parameters.localAxis, parameters.width);

	Info::ModelInfo modelInfo;
	modelInfo.vertices = &typedVertices;
	modelInfo.indices = &indices;
	modelInfo.texturePath = parameters.texture;

	setupModel(modelInfo, true);
	createPaths();
}

bool Road::isPointOnRoad(const Point& point) const
{
	auto endPoint = point - m_position;
	return polygonPointCollision(parameters.model, endPoint);
}


Point Road::getPointOnRoad(const Point& pointPosition) const
{
	const auto [pointOne, pointTwo] = findTwoClosestPoints(parameters.worldAxis, pointPosition);
	glm::vec3 directionPoint = pointTwo - pointOne;
	float lineLength = glm::length(directionPoint);

	float alpha = glm::acos(glm::dot(glm::normalize(pointTwo - pointOne), glm::normalize(pointPosition - pointOne)));
	float lineDistanceFromPointOne = glm::length(pointOne - pointPosition) * cos(alpha);

	// 
	const float minDistanceFromEnd = 0.5f;
	if (lineDistanceFromPointOne < minDistanceFromEnd)
	{
		return pointOne;
	}
	else
	{
		float percentageDistance = lineDistanceFromPointOne / lineLength;
		return pointOne + directionPoint * percentageDistance;
	}
}

Road::RoadParameters Road::getRoadParameters() const
{
	return parameters;
}

Points Road::getPath(int pathIndex) const
{
	//static std::mt19937_64 engine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	//std::uniform_int_distribution<int> distributor(0, parameters.laneCount - 1);
	auto path = parameters.lanes[pathIndex];

	for (auto& point : path)
		point += m_position;

	return path;


}

uint32_t Road::getLaneCount() const
{
	return parameters.laneCount;
}