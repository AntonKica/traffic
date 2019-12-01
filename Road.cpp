#include "Road.h"
#include "DataManager.h"

#include <glm/gtx/string_cast.hpp>
#include <numeric>


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

// interesting names
Point vectorIntersection(Point s1, Point e1, Point s2, Point e2)
{
	float a1 = e1.z - s1.z;
	float b1 = s1.x - e1.x;
	float c1 = a1 * s1.x + b1 * s1.z;

	float a2 = e2.z - s2.z;
	float b2 = s2.x - e2.x;
	float c2 = a2 * s2.x + b2 * s2.z;

	float delta = a1 * b2 - a2 * b1;

	return delta == 0 ? s2 : Point((b2 * c1 - b1 * c2) / delta, 0.0f, (a1 * c2 - a2 * c1) / delta);

	/*float a1 = e1.y - s1.y;
	float b1 = s1.x - e1.x;
	float c1 = a1 * s1.x + b1 * s1.y;

	float a2 = e2.y - s2.y;
	float b2 = s2.x - e2.x;
	float c2 = a2 * s2.x + b2 * s2.y;

	float delta = a1 * b2 - a2 * b1;
	//If lines are parallel, the result will be (NaN, NaN).
	return delta == 0 ? s1	: Point((b2 * c1 - b1 * c2) / delta, 0.0 , (a1 * c2 - a2 * c1) / delta);*/
}

Points createShape(const Points& points, int width)
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
			if (i - 1 >= 0)
			{
				previousPoint = points[i - 1];
			}
			if (i + 1 < points.size())
			{
				nextPoint = points[i + 1];
			}
			//update direction
			if (i + 1 < points.size())
				currentDirection = glm::normalize(nextPoint - curPoint);
			if (i - 1 >= 0)
				previousDirection = glm::normalize(curPoint - previousPoint);
			else
				previousDirection = currentDirection;

			glm::vec3 previousRightVec = glm::normalize(glm::cross(previousDirection, vectorUp)) * float(width) / 2.0f;
			glm::vec3 currentRightVec = glm::normalize(glm::cross(currentDirection, vectorUp)) * float(width) / 2.0f;


			Point right = vectorIntersection(previousPoint + previousRightVec, curPoint + previousRightVec,
				curPoint + currentRightVec, nextPoint + currentRightVec);

			Point left = vectorIntersection(previousPoint - previousRightVec, curPoint - previousRightVec,
				curPoint - currentRightVec, nextPoint - currentRightVec);

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
			if (i - 1 >= 0)
			{
				previousPoint = points[i - 1];
			}
			if (i + 1 < points.size())
			{
				nextPoint = points[i + 1];
			}
			if (i + 1 < points.size())
				currentDirection = glm::normalize(nextPoint - curPoint);
			if (i - 1 >= 0)
				previousDirection = glm::normalize(curPoint - previousPoint);
			else
				previousDirection = currentDirection;

			glm::vec3 previousRightVec = glm::normalize(glm::cross(previousDirection, vectorUp)) * float(width) / 2.0f;
			glm::vec3 currentRightVec = glm::normalize(glm::cross(currentDirection, vectorUp)) * float(width) / 2.0f;

			Point right = vectorIntersection(previousPoint + previousRightVec, curPoint + previousRightVec,
				curPoint + currentRightVec, nextPoint + currentRightVec);

			Point left = vectorIntersection(previousPoint - previousRightVec, curPoint - previousRightVec,
				curPoint - currentRightVec, nextPoint - currentRightVec);

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
	for (int i = 0; i < points.size(); ++i)
	{
			//update normal
		if (i + 1 < points.size())
			dirVec = glm::normalize(points[i + 1] - points[i]);

		glm::vec3 rightVec = glm::normalize(glm::cross(dirVec, vectorUp));

		const auto& curPoint = points[i];

		Point right = curPoint + rightVec * (0.5f);
		Point left = curPoint - rightVec * (0.5f);

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

std::pair<Point, Point> findTwoClosestPoints(const Points& points, const Point& point)
{
	std::optional<Point> first, second;
	for (const auto& point_ : points)
	{
		if (!first)
		{
			first = point_;
			continue;
		}

		float currentDistance = glm::length(point - point_);
		float firstPointDistance = glm::length(point - first.value());
		if (currentDistance < firstPointDistance)
		{
			second = first;
			first = point_;
		}
		else if (!second)
		{
			second = point_;
		}
		else if (float secondPointDistance = glm::length(point - second.value());
			currentDistance < secondPointDistance)
		{
			second = point_;
		}
	}

	return std::make_pair(first.value(), second.value());
};

template<class C, class T1, class T2, class T3> int insertElemementBetween(C& container, T1 first, T2 second, T3 element)
{
	for (int pointIndex = 0; pointIndex + 1 < container.size(); ++pointIndex)
	{
		if ((container[pointIndex] == first && container[pointIndex + 1] == second) ||
			(container[pointIndex + 1] == first && container[pointIndex] == second))
		{
			container.insert(std::begin(container) + pointIndex + 1, element);
			container.erase(std::unique(std::begin(container), std::end(container)), std::end(container));
			return pointIndex + 1;
		}
	}


	std::throw_with_nested("Supplied range, which doesnt exist in supplied container!");
}

std::pair<Road, std::optional<Road>> Road::splitRoad(const Point& splitPoint)
{
	auto localSplitPoint = splitPoint - m_position;

	Road firstSplit;
	std::optional<Road> optionalSplit;

	if (glm::length(localSplitPoint - parameters.axis.front()) <= 0.01 || glm::length(localSplitPoint - parameters.axis.back()) <= 0.01)
	{
		firstSplit = *this;
	}
	else
	{
		const auto[firstPoint, secondPoint] = findTwoClosestPoints(parameters.axis, localSplitPoint);

		Points newAxis = parameters.axis;
		int splitIndex = insertElemementBetween(newAxis, firstPoint, secondPoint, localSplitPoint);

		auto positionTransform = [&](const Point& point) { return point + m_position; };
		Points firstSplitAxis;
		std::transform(std::begin(newAxis), std::begin(newAxis) + splitIndex + 1, std::back_inserter(firstSplitAxis), positionTransform);
		Points secondSplitAxis;
		std::transform(std::begin(newAxis) + splitIndex, std::end(newAxis), std::back_inserter(secondSplitAxis), positionTransform);


		firstSplit.construct(firstSplitAxis, parameters.laneCount, parameters.width, parameters.texture);

		Road optRoad;
		optRoad.construct(secondSplitAxis, parameters.laneCount, parameters.width, parameters.texture);
		optionalSplit = optRoad;

	}

	return std::make_pair(firstSplit, optionalSplit);
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
	auto& axisOne = firstRoad.parameters.axis;
	auto& axisTwo = secondRoad.parameters.axis;

	//std::set_intersection(std::begin(axisOne), std::end(axisOne), std::begin(axisTwo), std::end(axisTwo))
	Point intersection;
	int intersectionCount = 0;
	for (const Point& p1 : axisOne)
	{
		for (const Point& p2 : axisTwo)
		{
			if (p1 + firstRoad.m_position == p2 + secondRoad.m_position)
			{
				if(intersectionCount == 0)
					intersection = p1;
				++intersectionCount;
				break;
			}
		}
	}

	if (intersection != axisOne.back())
	{
		std::reverse(std::begin(axisOne), std::end(axisOne));
	}

	if (intersection + firstRoad.m_position != axisTwo.front() + secondRoad.m_position)
	{
		std::reverse(std::begin(axisTwo), std::end(axisTwo));
	}

	//glm::vec3 averagePos = (m_position - road.m_position) / 2.0f;

//	Points curvePoints = generateCurveFromThreePoints({ axisOne[axisOne.size() - 2] + firstRoad.m_position, axisOne[axisOne.size() - 1] + firstRoad.m_position, axisTwo[1] + secondRoad.m_position }, 3);
	Points concatedAxis;
	std::transform(std::begin(axisOne), std::end(axisOne), std::back_inserter(concatedAxis),
		[&firstRoad](const Point& point) {	return point + firstRoad.m_position;	});
	//concatedAxis.insert(concatedAxis.end(), std::begin(curvePoints), std::end(curvePoints));
	std::transform(std::begin(axisTwo) + 1, std::end(axisTwo), std::back_inserter(concatedAxis),
		[&secondRoad](const Point& point) {	return point + secondRoad.m_position;	});

	if (intersectionCount == 2)
		concatedAxis.insert(concatedAxis.end(), *(concatedAxis.begin() + 1));

	Road concatedRoad;
	concatedRoad.construct(concatedAxis, firstRoad.parameters.laneCount, firstRoad.parameters.width, firstRoad.parameters.texture);

	return concatedRoad;
}

Points Road::createLocalPoints(const Points& points, const glm::vec3& position)
{
	Points localPoints(points.size());
	std::transform(std::begin(points), std::end(points), std::begin(localPoints),
		[&position](const Point& point)
		{
			return point - position;
		});

	return localPoints;
}


void Road::construct(const Points& points, uint32_t laneCount, float width, std::string texture)
{
	m_position = getAveratePosition(points);

	parameters.axis = createLocalPoints(points, m_position);
	parameters.model = createShape(points, width);
	parameters.laneCount = laneCount;
	parameters.width = width;
	parameters.texture = texture;

	const auto [typedVertices, indices] = createTexturedShape(parameters.axis, parameters.width);

	Info::ModelInfo modelInfo;
	modelInfo.vertices = &typedVertices;
	modelInfo.indices = &indices;
	modelInfo.texturePath = parameters.texture;

	setupModel(modelInfo, true);
}

void Road::construct(const Points& points, glm::vec3 position, uint32_t laneCount, float width, std::string texture)
{
	m_position = position;

	parameters.axis = points;
	parameters.model = createShape(points, width);
	parameters.laneCount = laneCount;
	parameters.width = width;
	parameters.texture = texture;

	const auto [typedVertices, indices] = createTexturedShape(parameters.axis, parameters.width);

	Info::ModelInfo modelInfo;
	modelInfo.vertices = &typedVertices;
	modelInfo.indices = &indices;
	modelInfo.texturePath = parameters.texture;

	setupModel(modelInfo, true);
}

bool Road::isPointOnRoad(const Point& point) const
{
	return polygonPointCollision(parameters.model, point);
}


Point Road::getPointOnRoad(const Point& pointPosition) const
{
	// in local coordinates
	glm::vec3 localPosition = pointPosition - m_position;

	const auto [pointOne, pointTwo] = findTwoClosestPoints(parameters.axis, localPosition);
	glm::vec3 directionPoint = pointTwo - pointOne;
	float lineLength = glm::length(directionPoint);

	float alpha = glm::acos(glm::dot(glm::normalize(pointTwo - pointOne), glm::normalize(localPosition - pointOne)));
	float lineDistanceFromPointOne = glm::length(pointOne - localPosition) * cos(alpha);

	// 
	const float minDistanceFromEnd = 0.5f;
	if (lineDistanceFromPointOne < minDistanceFromEnd)
	{
		return m_position + pointOne;
	}
	else
	{
		float percentageDistance = lineDistanceFromPointOne / lineLength;
		return m_position + pointOne + directionPoint * percentageDistance;
	}
	/*
	float linearCoeficient = (pointOne.z - pointTwo.z) / (pointOne.x - pointTwo.x);
	float absoluteMember = pointOne.z - linearCoeficient * pointOne.x;
	*/

}

Road::RoadParameters Road::getRoadParameters() const
{
	return parameters;
}
