#include "Road.h"
#include "Utilities.h"
#include "Mesh.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
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

template <class PointsType> auto //std::tuple<typename PointsType::iterator, typename PointsType::iterator, float> 
	travellDistanceOnPoints(PointsType& points, float distance)
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
	for (auto vert = polygon.begin(); vert != polygon.end(); ++vert)
	{
		auto nextVert = (vert + 1) ==
			polygon.end() ? polygon.begin() : vert + 1;

		// z test
		if (((vert->z > point.z) && nextVert->z < point.z)
			|| (vert->z < point.z && (nextVert->z >point.z)))
		{
			if (point.x < (nextVert->x - vert->x) * (point.z - vert->z) / (nextVert->z - vert->z) + vert->x)
			{
				collision = !collision;
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
	glm::vec3 currentDirectionPoint;
	glm::vec3 previousDirectionPoint;
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
				currentDirectionPoint = glm::normalize(nextPoint - curPoint);
			}
			else if (points.front() == points.back())
			{
				nextPoint = *(points.begin() + 1);
				currentDirectionPoint = glm::normalize(nextPoint - curPoint);
			}

			if (i - 1 >= 0)
			{
				previousPoint = points[i - 1];
				previousDirectionPoint = glm::normalize(curPoint - previousPoint);
			}
			else if (points.front() == points.back())
			{
				previousPoint = *(points.end() - 2);
				previousDirectionPoint = glm::normalize(curPoint - previousPoint);
			}
			else
			{
				previousDirectionPoint = currentDirectionPoint;
			}

			const auto [left, right] = getSidePoints(previousDirectionPoint, currentDirectionPoint, previousPoint, curPoint, nextPoint, width);

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
	glm::vec3 currentDirectionPoint;
	glm::vec3 previousDirectionPoint;
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
				currentDirectionPoint = glm::normalize(nextPoint - curPoint);
			}
			else if (points.front() == points.back())
			{
				nextPoint = *(points.begin() + 1);
				currentDirectionPoint = glm::normalize(nextPoint - curPoint);
			}

			if (i - 1 >= 0)
			{
				previousPoint = points[i - 1];
				previousDirectionPoint = glm::normalize(curPoint - previousPoint);
			}
			else if (points.front() == points.back())
			{
				previousPoint = *(points.end() - 2);
				previousDirectionPoint = glm::normalize(curPoint - previousPoint);
			}
			else
			{
				previousDirectionPoint = currentDirectionPoint;
			}

			const auto [left, right] = getSidePoints(previousDirectionPoint, currentDirectionPoint, previousPoint, curPoint, nextPoint, width);

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
	glm::vec3 currentDirectionPoint;
	glm::vec3 previousDirectionPoint;
	Point previousPoint;
	Point nextPoint;
	for (int i = 0; i < points.size(); ++i)
	{
		const auto& curPoint = points[i];
		previousPoint = curPoint;
		if (i + 1 < points.size())
		{
			nextPoint = points[i + 1];
			currentDirectionPoint = glm::normalize(nextPoint - curPoint);
		}
		else if (points.front() == points.back())
		{
			nextPoint = *(points.begin() + 1);
			currentDirectionPoint = glm::normalize(nextPoint - curPoint);
		}

		if (i - 1 >= 0)
		{
			previousPoint = points[i - 1];
			previousDirectionPoint = glm::normalize(curPoint - previousPoint);
		}
		else if (points.front() == points.back())
		{
			previousPoint = *(points.end() - 2);
			previousDirectionPoint = glm::normalize(curPoint - previousPoint);
		}
		else
		{
			previousDirectionPoint = currentDirectionPoint;
		}

		const auto [left, right] = getSidePoints(previousDirectionPoint, currentDirectionPoint, previousPoint, curPoint, nextPoint, outlineSize);
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

bool arePointsInRange(const Point& p1, const Point& p2, float range)
{
	return glm::length(p1 - p2) <= range;
}

VD::Indices createTriangledIndices(const Points& points)
{
	const auto triangleCount = points.size() * 3 - 2;
	VD::Indices indices(triangleCount);

	auto indicesIter = indices.begin();
	for (uint32_t index = 0; index + 2 < points.size(); ++index)
		indicesIter = indices.insert(indicesIter, { index, index + 1, index + 2 });

	return indices;
}

std::optional<SegmentedShape::Segment> SegmentedShape::selectSegment(Point arbitraryPoint) const
{
	std::optional<Segment> selectedSegment;

	for (int index = 0; index + 1 < m_joints.size(); ++index)
	{
		Points vertices = {
			m_joints[index].left, m_joints[index + 1].left,
			m_joints[index + 1].right, m_joints[index].right };

		if (polygonPointCollision(vertices, arbitraryPoint))
		{
			Segment segment;
			segment.start = &m_joints[index];
			segment.end = &m_joints[index + 1];

			selectedSegment = segment;
			break;
		}
	}
	return selectedSegment;
}

std::vector<SegmentedShape::Segment> SegmentedShape::getJointSegments(Shape::AxisPoint jointPoint) const
{
	std::vector<SegmentedShape::Segment> segments;
	for (int index = 0; index < m_joints.size(); ++index)
	{
		if (m_joints[index].centre == jointPoint)
		{
			// special case of circularity
			if (index == 0 && isCirculary())
			{
				Segment s1, s2;
				s1.start = &*(m_joints.end() - 2);
				s1.end = &*(m_joints.end() - 1);

				s2.start = &*(m_joints.begin());
				s2.end = &*(m_joints.begin() + 1);

				segments.insert(segments.begin(), { s1, s2 });
			}
			if (index >= 1)
			{
				Segment s;
				s.start = &m_joints[index - 1];
				s.end = &m_joints[index];

				segments.emplace_back(s);
			}
			if (index + 1 < m_joints.size())
			{
				Segment s;
				s.start = &m_joints[index];
				s.end = &m_joints[index + 1];

				segments.emplace_back(s);
			}
		}
	}
	return segments;
}

std::optional<Shape::AxisPoint> SegmentedShape::getShapeAxisPoint(Point arbitraryPoint) const
{
	std::optional<Shape::AxisPoint> axisPoint;

	auto selectedSegment = selectSegment(arbitraryPoint);
	if (selectedSegment)
	{
		const auto& [start, end] = selectedSegment.value();
		Point shapePoint = getClosestPointToLine<Point>(start->centre, end->centre, arbitraryPoint);
		// dont oveerlap actual shape and snap to joint
		{
			/*std::cout << glm::length(Point(shapePoint));
			if (glm::length(Point(shapePoint)) > 100.0f)
				std::cout << "Woah\n";*/

			const float minDistanceFromJoint = m_width / 2.0f;
			const float shapeLength = glm::length(start->centre - end->centre);

			auto distanceFromStart = glm::length(shapePoint - start->centre);
			auto distanceFromEnd = glm::length(shapePoint - end->centre);
			if (distanceFromStart < minDistanceFromJoint || distanceFromEnd > shapeLength)
				shapePoint = start->centre;
			else if (distanceFromEnd < minDistanceFromJoint || distanceFromStart > shapeLength)
				shapePoint = end->centre;
		}

		const float minDistanceFromEndPoints = m_width;
		if (auto head = getHead(), tail = getTail(); (arePointsInRange(head, shapePoint, minDistanceFromEndPoints) ||
			arePointsInRange(tail, shapePoint, minDistanceFromEndPoints)) 
			&& !isCirculary())
			axisPoint = glm::length(head - arbitraryPoint) < glm::length(tail - arbitraryPoint) ? head : tail;
		else
			axisPoint.emplace(shapePoint);
	}

	return axisPoint;
}

Mesh SegmentedShape::createMesh(const SegmentedShape& shape)
{
	auto& joints = shape.getShape();
	const auto verticesCount = joints.size() * 2;

	VD::PositionVertices vertices(verticesCount);
	VD::TextureVertices textureCoords(verticesCount);

	auto verticesIter = vertices.begin();
	auto texturesIter = textureCoords.begin();
	float travelledLength = 0;
	Point previousTravellPoint = joints[0].centre;
	for (const auto& joint : joints)
	{
		travelledLength += glm::length(previousTravellPoint - joint.centre);
		previousTravellPoint = joint.centre;

		*verticesIter++ = joint.left;
		*verticesIter++ = joint.right;

		VD::TextureVertex left, right;
		left.x = 1.0;
		left.y = travelledLength;
		right.x = 0.0;
		right.y = travelledLength;

		*texturesIter++ = left;
		*texturesIter++ = right;
	}
	VD::Indices indices = createTriangledIndices(vertices);

	Mesh mesh;
	mesh.vertices.positions = vertices;
	mesh.vertices.textures= textureCoords;
	mesh.indices = indices;

	return mesh;
}

const SegmentedShape::Joints& SegmentedShape::getShape() const
{
	return m_joints;
}

Points SegmentedShape::getSkeleton()  const
{
	Points skeleton(m_joints.size() * 2);

	auto skeletonIter = skeleton.begin();
	for (const auto& joint : m_joints)
	{
		*skeletonIter++ = joint.left;
		*skeletonIter++ = joint.right;
	}

	return skeleton;
}

Shape::Axis SegmentedShape::getAxis() const
{
	Shape::Axis axis(m_joints.size());

	auto axisIter = axis.begin();
	for (const auto& joint : m_joints)
		*axisIter++ = joint.centre;

	return axis;
}

Shape::AxisPoint  SegmentedShape::getHead() const
{
	return m_joints.back().centre;
}

Shape::AxisPoint SegmentedShape::getTail() const
{
	return m_joints.front().centre;
}

bool SegmentedShape::sitsOnHead(const Point& point) const
{
	return !isCirculary() && approxSamePoints(point, getHead());
}

bool SegmentedShape::sitsOnTail(const Point& point) const
{
	return !isCirculary() && approxSamePoints(point, getTail());
}

bool SegmentedShape::sitsOnTailOrHead(const Point& point) const
{
	return sitsOnTail(point) || sitsOnHead(point);
}

bool SegmentedShape::sitsOnShape(const Point& point) const
{
	return selectSegment(point).has_value();
	/*for (int index = 0; index + 1 < m_joints.size(); ++index)
	{
		Points vertices = {
			m_joints[index].side.left, m_joints[index + 1].side.left, 
			m_joints[index + 1].side.right, m_joints[index].side.right};
		if (polygonPointCollision(vertices, point))
			return true;
	}
	return false;*/
}

bool SegmentedShape::sitsOnAxis(const Point& point) const
{
	for (int index = 0; index + 1 < m_joints.size(); ++index)
	{
		Points line = { m_joints[index].centre, m_joints[index + 1].centre };
		if (pointSitsOnLine(line[0], line[1], point))
			return true;
	}
	return false;
}

bool SegmentedShape::sitsOnAnySegmentCorner(const Point& point) const
{
	for (int index = 0; index < m_joints.size(); ++index)
	{
		if (approxSamePoints(m_joints[index].centre, point))
			return true;
	}
	return false;
}

bool SegmentedShape::isCirculary() const
{
	return approxSamePoints(getHead(), getTail());
}

void SegmentedShape::reverseBody()
{
	std::reverse(std::begin(m_joints), std::end(m_joints));
	for (auto& joint : m_joints)
		std::swap(joint.left, joint.right);

	// quiteVerbose
	if (!m_endDirectionPoints.empty())
	{
		if (m_endDirectionPoints.size() == 1)
		{
			if (auto connection = m_endDirectionPoints.find(TAIL); connection != m_endDirectionPoints.end())
			{
				m_endDirectionPoints[HEAD] = m_endDirectionPoints[TAIL];
				m_endDirectionPoints.erase(TAIL);
			}
			else
			{
				m_endDirectionPoints[TAIL] = m_endDirectionPoints[HEAD];
				m_endDirectionPoints.erase(HEAD);
			}
		}
		else
		{
			std::swap(m_endDirectionPoints[HEAD], m_endDirectionPoints[TAIL]);
		}
	}
}

void SegmentedShape::setTailDirectionPoint(Point point)
{
	m_endDirectionPoints[TAIL] = point;
}

void SegmentedShape::removeTailDirectionPoint()
{
	if(hasTailDirectionPoint())
		m_endDirectionPoints.erase(TAIL);
}

Point SegmentedShape::getTailDirectionPoint() const
{
	return m_endDirectionPoints.find(TAIL)->second;
}

bool SegmentedShape::hasTailDirectionPoint() const
{
	return m_endDirectionPoints.find(TAIL) != m_endDirectionPoints.end();
}

void SegmentedShape::setHeadDirectionPoint(Point point)
{
	m_endDirectionPoints[HEAD] = point;
}

void SegmentedShape::removeHeadDirectionPoint()
{
	if(hasHeadDirectionPoint())
		m_endDirectionPoints.erase(HEAD);
}

Point SegmentedShape::getHeadDirectionPoint() const
{
	return m_endDirectionPoints.find(HEAD)->second;
}

bool SegmentedShape::hasHeadDirectionPoint() const
{
	return m_endDirectionPoints.find(HEAD) != m_endDirectionPoints.end();
}


void SegmentedShape::clearDirectionPoints()
{
	m_endDirectionPoints.clear();
}

void SegmentedShape::construct(const Shape::Axis& axisPoints, float width)
{
	Points points(std::begin(axisPoints), std::end(axisPoints));

	construct(points, width);
}

void SegmentedShape::construct(const Points& constructionPoints, float width)
{
	OrientedConstructionPoints cp{};
	if (hasTailDirectionPoint()) cp.tailDirectionPoint = getTailDirectionPoint();
	cp.points = constructionPoints;
	if (hasHeadDirectionPoint()) cp.headDirectionPoint = getHeadDirectionPoint();

	construct(cp, width);
}

void SegmentedShape::construct(const OrientedConstructionPoints& constructionPoints, float width)
{
	if (constructionPoints.tailDirectionPoint)
		setTailDirectionPoint(constructionPoints.tailDirectionPoint.value());
	if (constructionPoints.headDirectionPoint)
		setHeadDirectionPoint(constructionPoints.headDirectionPoint.value());

	m_width = width;
	createShape(constructionPoints.points);
}

void SegmentedShape::reconstruct()
{
	auto axis = getAxis();
	Points points(std::begin(axis), std::end(axis));

	construct(points, m_width);
}
\

void SegmentedShape::mergeWith(const SegmentedShape& otherShape)
{	
	auto thisShapeAxis = getAxis();
	auto otherShapeAxis = otherShape.getAxis();
	// reverse the right axis since we copy o
	if (approxSamePoints(getHead(), otherShape.getHead()))
	{
		std::reverse(std::begin(otherShapeAxis), std::end(otherShapeAxis));
	}
	else if (approxSamePoints(getTail(), otherShape.getTail()))
	{
		std::reverse(std::begin(thisShapeAxis), std::end(thisShapeAxis));
	}
	// copy the right direction
	if (approxSamePoints(thisShapeAxis.front(), otherShapeAxis.back()))
	{
		thisShapeAxis.insert(std::begin(thisShapeAxis),
			std::begin(otherShapeAxis), std::end(otherShapeAxis) - 1);

		// connections
		removeTailDirectionPoint();
		if (otherShape.hasTailDirectionPoint())
			setTailDirectionPoint(otherShape.getTailDirectionPoint());

		construct(thisShapeAxis, m_width);
	}
	else
	{
		std::copy(std::begin(otherShapeAxis) + 1, std::end(otherShapeAxis), std::back_inserter(thisShapeAxis));

		// connections
		removeHeadDirectionPoint();
		if (otherShape.hasHeadDirectionPoint())
			setHeadDirectionPoint(otherShape.getHeadDirectionPoint());

		construct(thisShapeAxis, m_width);
	}
}

std::optional<SegmentedShape> SegmentedShape::split(Shape::AxisPoint splitPoint)
{
	std::optional<SegmentedShape> optionalSplit;

	if (!isCirculary())
	{
		auto newAxis = getAxis();
		const auto [firstPoint, secondPoint] = getEdgesOfAxisPoint(splitPoint);

		auto newSplitIter = insertElemementBetween(newAxis, firstPoint, secondPoint, splitPoint);
		//dont erase if same 
		// recteate this worldAxis
		Shape::Axis curSplitAxis (std::begin(newAxis), newSplitIter + 1);
		Shape::Axis secondSplitAxis(newSplitIter, std::end(newAxis));


		SegmentedShape optShape;
		// sort connections
		if (hasHeadDirectionPoint())
		{
			optShape.setHeadDirectionPoint(getHeadDirectionPoint());
			removeHeadDirectionPoint();
		}
		construct(curSplitAxis, m_width);
		optShape.construct(secondSplitAxis, m_width);
		optionalSplit = optShape;
	}
	else //if ()
	{
		// move axis to splitPoints
		auto newAxis = getAxis();

		// remove uniquePoints
		const auto [firstPoint, secondPoint] = selectSegment(splitPoint).value();
		if (secondPoint->centre == getHead())
			newAxis.erase(newAxis.begin());
		else //if (firstPoint->centre == getTail() || secondPoint->centre == getTail())
			newAxis.pop_back();

		if (approxSamePoints(splitPoint, firstPoint->centre))
			setNewCircularEndPoints(firstPoint->centre);
		else if (approxSamePoints(splitPoint, secondPoint->centre))
			setNewCircularEndPoints(secondPoint->centre);
		else
		{
			insertElemementBetween(newAxis, firstPoint->centre, secondPoint->centre, splitPoint);
			construct(newAxis, m_width);
			setNewCircularEndPoints(splitPoint);
			auto axis = getAxis();
		}
	}

	return optionalSplit;
}

Shape::AxisPoint SegmentedShape::shorten(Shape::AxisPoint shapeEnd, float size)
{
	Shape::Axis newAxis;
	Shape::AxisPoint shortenPosition = {};
	if (sitsOnTail(shapeEnd))
	{
		removeTailDirectionPoint();
		auto axis = getAxis();
		auto [firstPoint, secondPoint, travelledDistance] = travellDistanceOnPoints(axis, size);
		if (secondPoint == std::end(axis))
			throw std::runtime_error("Too far size to shorten");

		float newVecLength = travelledDistance - size;
		if (newVecLength != 0)
		{
			// reverse direction and go from second point
			glm::vec3 direction = glm::normalize(*firstPoint - *secondPoint);
			auto pointPos = *secondPoint + (direction * newVecLength);

			// overwrite
			shortenPosition = *firstPoint = Shape::AxisPoint(pointPos);
		}

		// remove unsuitable points
		axis.erase(axis.begin(), firstPoint);
		newAxis = axis;
	}
	else
	{
		removeHeadDirectionPoint();
		reverseBody();
		//reversed 
		auto reversedAxis = getAxis();
		auto [firstPoint, secondPoint, travelledDistance] = travellDistanceOnPoints(reversedAxis, size);

		if (firstPoint == std::end(reversedAxis))
			throw std::runtime_error("Too far size to shorten");

		float newVecLength = travelledDistance - size;
		if (newVecLength != 0)
		{
			// reverse direction and go from second point
			glm::vec3 direction = glm::normalize(*firstPoint - *secondPoint);
			auto pointPos = *secondPoint + (direction * newVecLength);

			// overwrite
			shortenPosition = *firstPoint = Shape::AxisPoint(pointPos);
		}

		// remove unsuitable points
		auto copyIt = std::reverse_copy(firstPoint, std::end(reversedAxis), std::back_insert_iterator(newAxis));
	}

	construct(newAxis, m_width);
	return shortenPosition;
}

SegmentedShape::ShapeCut SegmentedShape::getShapeCut(Shape::AxisPoint axisPoint, float radius) const
{
	auto axis = getAxis();
	if (isCirculary())
	{
		setNewCirclePointsStart(axis, axisPoint);
	}
	
	// insert if unique
	if (std::find(std::begin(axis), std::end(axis), axisPoint) == std::end(axis))
	{
		auto [firstPoint, secondPoint] = getEdgesOfAxisPoint(axisPoint);
		insertElemementBetween(axis, firstPoint, secondPoint, axisPoint);
	}

	float halfRadius = radius / 2.0f;
	auto [startTrail, remainingFromStart] = travellDistanceOnTrailFromPointInDirection(halfRadius, axis, axisPoint, true);
	auto [endTrail, remainingFromEnd] = travellDistanceOnTrailFromPointInDirection(halfRadius, axis, axisPoint, false);
	// correcting, nothing else
	if (remainingFromStart)
	{
		float distance = halfRadius + remainingFromStart.value();
		auto [newEndTrail, _] = travellDistanceOnTrailFromPointInDirection(distance, axis, axisPoint, false);

		endTrail = newEndTrail;
	}
	else if(remainingFromEnd)
	{
		float distance = halfRadius + remainingFromEnd.value();
		auto [newStartTrail, _] = travellDistanceOnTrailFromPointInDirection(distance, axis, axisPoint, true);

		startTrail = newStartTrail;
	}

	// copy result
	Shape::Axis cutAxis(startTrail.size() - 1 + endTrail.size());
	auto copyIt = std::reverse_copy(std::begin(startTrail), std::end(startTrail), std::begin(cutAxis));
	copyIt = std::copy(std::begin(endTrail) + 1, std::end(endTrail), copyIt);

	SegmentedShape::ShapeCut cut{ cutAxis };

	return cut;
}

Shape::AxisSegment SegmentedShape::getEdgesOfAxisPoint(Shape::AxisPoint axisPoint) const
{

	for (int index = 0; index + 1 < m_joints.size(); ++index)
	{
		Shape::AxisSegment axisSegment = { m_joints[index].centre, m_joints[index + 1].centre };
		if (pointSitsOnLine(axisSegment[0], axisSegment[1], axisPoint))
			return axisSegment;
	}

	throw std::runtime_error("Suppleid non axis point for edges!");
}

std::optional<SegmentedShape> SegmentedShape::cut(ShapeCut cutPoints)
{
	std::optional<SegmentedShape> optShape;
	auto axis = getAxis();
	// we assume that point are in the same direction
	if (!isCirculary())
	{
		// we dont need cut
		auto cut = split(cutPoints.axis.front()).value();

		// next point is on the other shape
		auto newShape = cut.split(cutPoints.axis.back()).value();
		if (hasHeadDirectionPoint())
		{
			newShape.setHeadDirectionPoint(getHeadDirectionPoint());
			removeHeadDirectionPoint();
		}

		optShape = newShape;
	}
	else
	{
		// circle not implemented yet
	}

	return optShape;
}

void SegmentedShape::setNewCircularEndPoints(Shape::AxisPoint axisPoint)
{
	auto newAxis = getAxis();
	setNewCirclePointsStart(newAxis, axisPoint);

	construct(newAxis, m_width);
}

void SegmentedShape::eraseCommonPoints()
{
	auto current = m_joints.begin();
	while (current != m_joints.end())
	{
		auto cursor = current + 1;
		while (cursor != m_joints.end())
		{
			if (approxSamePoints(current->centre, cursor->centre))
			{
				cursor = m_joints.erase(cursor);
			}
			else
			{
				++cursor;
			}
		}

		++current;
	}
}

void SegmentedShape::createShape(const Points& axis)
{
	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);
	// mke space
	m_joints.resize(axis.size());

	glm::vec3 dirVec;
	glm::vec3 currentDirection;
	glm::vec3 previousDirection;
	Point previousPoint;
	Point nextPoint;
	for (int i = 0; i < m_joints.size(); ++i)
	{
		auto& currentJoint = m_joints[i];
		// vertices
		{
			const auto& curPoint = axis[i];
			previousPoint = curPoint;
			if (i + 1 < axis.size())
			{
				nextPoint = axis[i + 1];
				currentDirection = glm::normalize(nextPoint - curPoint);
			}
			else if (hasHeadDirectionPoint())
			{
				nextPoint = getHeadDirectionPoint();
				currentDirection = glm::normalize(nextPoint - curPoint);
			}
			else if (axis.front() == axis.back())
			{
				nextPoint = *(axis.begin() + 1);
				currentDirection = glm::normalize(nextPoint - curPoint);
			}

			if (i - 1 >= 0)
			{
				previousPoint = axis[i - 1];
				previousDirection = glm::normalize(curPoint - previousPoint);
			}
			else if (hasTailDirectionPoint())
			{
				previousPoint = getTailDirectionPoint();
				previousDirection = glm::normalize(curPoint - previousPoint);
			}
			else if (axis.front() == axis.back())
			{
				previousPoint = *(axis.end() - 2);
				previousDirection = glm::normalize(curPoint - previousPoint);
			}
			else
			{
				previousDirection = currentDirection;
			}
			
			const auto [left, right] = getSidePoints(previousDirection, currentDirection, previousPoint, curPoint, nextPoint, m_width);
			Joint newJoint(left, Shape::AxisPoint(curPoint), right);
	
			currentJoint = newJoint;
		}
	}
}

Road::RoadParameters Road::getParameters() const
{
	return m_parameters;
}

bool Road::sitsOnEndPoints(const Point& point) const
{
	return m_shape.sitsOnTailOrHead(point);
}

bool Road::sitsOnRoad(const Point& point) const
{
	return m_shape.sitsOnShape(point);
}

Shape::AxisPoint Road::getPointOnRoad(const Point& point)
{
	return m_shape.getShapeAxisPoint(point).value();
}

void Road::construct(Shape::Axis axisPoints, uint32_t laneCount, float width, std::string texture)
{
	Points points(std::begin(axisPoints), std::end(axisPoints));

	construct(points, laneCount, width, texture);
}

void Road::construct(Points creationPoints, uint32_t laneCount, float width, std::string texture)
{
	RoadParameters parameters;
	parameters.laneCount = laneCount;
	parameters.width = width;
	parameters.texture = texture;

	construct(creationPoints, parameters);
}

void Road::construct(Points creationPoints, const RoadParameters& parameters)
{
	m_position = {};

	m_parameters = parameters;

	SegmentedShape::OrientedConstructionPoints cps;
	cps.points = creationPoints;
	for (const auto& cp : m_connections)
	{
		if (approxSamePoints(creationPoints.front(), cp.point))
			cps.tailDirectionPoint = cp.connected->getDirectionPointFromConnectionPoint(creationPoints.front());
		else
			cps.headDirectionPoint = cp.connected->getDirectionPointFromConnectionPoint(creationPoints.back());
	}
	m_shape.construct(cps, m_parameters.width);

	// model
	auto mesh = SegmentedShape::createMesh(m_shape);
	mesh.textures[VD::TextureType::DIFFUSE] = m_parameters.texture;

	Model model;
	model.meshes.push_back(mesh);

	Info::ModelInfo modelInfo;
	modelInfo.model = &model;

	setupModel(modelInfo, true);
	createPaths();
}

void Road::reconstruct()
{
	auto axis = m_shape.getAxis();
	Points points(std::begin(axis), std::end(axis));

	construct(points, m_parameters);
}

void Road::mergeWith(Road& road)
{
	m_shape.mergeWith(road.m_shape);
	road.transferConnections(this);
	reconstruct();
}
Road::SplitProduct Road::split(Shape::AxisPoint splitPoint)
{

	Road::SplitProduct product;
	if (auto optSplit = m_shape.split(splitPoint))
	{
		Road road;
		// // validate connections
		for(auto& connection : m_connections)
		{
			if (!sitsOnEndPoints(connection.point))
			{
				product.connection = connection;
				dismissConnection(connection);
			}
		}
		// then construct
		road.construct(optSplit.value().getAxis(), m_parameters.laneCount, m_parameters.width, m_parameters.texture);

		product.road = road;
	}
	// reconstruct t the end, may be bettter performance, who knows
	reconstruct();

	return product;
}


Shape::AxisPoint Road::shorten(Shape::AxisPoint roadEnd, float size)
{
	auto shortPoint = m_shape.shorten(roadEnd, size);
	reconstruct();
	return shortPoint;
}

SegmentedShape::ShapeCut Road::getCut(Shape::AxisPoint roadAxisPoint) const
{
	return m_shape.getShapeCut(roadAxisPoint, m_parameters.width);
}

Road::CutProduct Road::cut(SegmentedShape::ShapeCut cutPoints)
{
	Road::CutProduct product;
	if (auto optSplit = m_shape.cut(cutPoints))
	{
		Road road;
		// // validate connections
		for (auto& connection : m_connections)
		{
			if (!sitsOnEndPoints(connection.point))
			{
				product.connection = connection;
				dismissConnection(connection);
			}
		}
		// then construct
		road.construct(optSplit.value().getAxis(), m_parameters.laneCount, m_parameters.width, m_parameters.texture);

		product.road = road;
	}

	reconstruct();

	return product;
}

BasicRoad::ConnectionPossibility Road::canConnect(Line connectionLine, Shape::AxisPoint connectionPoint) const
{
	ConnectionPossibility connectionPossibility{};
	//reored 
	if (approxSamePoints(connectionLine[0], connectionPoint))
		std::swap(connectionLine[0], connectionLine[1]);

	// sits on corner?
	if (m_shape.sitsOnAnySegmentCorner(connectionPoint))
	{
		auto segments = m_shape.getJointSegments(connectionPoint);

		// end or begin
		if (segments.size() == 1)
		{
			auto [start, end] = segments[0];
			// end == end of shape
			if (approxSamePoints(connectionPoint, start->centre))
				std::swap(start, end);

			glm::vec3 axisDir = glm::normalize(end->centre - start->centre);
			glm::vec3 endToLineEnd = glm::normalize(connectionLine[0] - end->centre);
			float connectionAngle = glm::degrees(glm::acos(glm::dot(axisDir, endToLineEnd)));

			if (connectionAngle <= 90.0f)
			{
				connectionPossibility.canConnect = true;
			}
			else
			{

			}
		}
		else
		{
			auto& segment1 = segments[0];
			auto& segment2 = segments[1];

			// supose we know segment 1s end is same as segment2s start
			if (segment1.end != segment2.start)
				std::swap(segment1, segment2);
			// segment end / start = connection point
			auto calcAngle = [](const glm::vec3& v1, const glm::vec3& v2)
			{
				auto dot = v1.x * v2.x + v1.z * v2.z;
				auto det = v1.x * v2.z - v1.z * v2.x;
				auto res = std::atan2(det, dot);

				return res > 0 ? res : res + glm::two_pi<float>();
			};

			glm::vec3 conLineDir = glm::normalize(connectionLine[0] - connectionPoint);
			glm::vec3 dirS1 = glm::normalize(segment1.start->centre - connectionPoint);
			glm::vec3 dirS2 = glm::normalize(segment2.end->centre - connectionPoint);
			float roadAngle = calcAngle(dirS1, dirS2);
			float angleLineS1 = calcAngle(conLineDir, dirS1);
			// direction matters
			float angleLineS2 = calcAngle(dirS2, conLineDir);

			if (angleLineS1 > roadAngle || angleLineS2 > roadAngle)
			{
				connectionPossibility.canConnect = true;
			}
		}
	}
	else // sits between corners/ joints
	{
		auto [start, end] = m_shape.selectSegment(connectionPoint).value();

		const glm::vec3 axisDir = glm::normalize(end->centre - start->centre);
		const glm::vec3 conLineDir = glm::normalize(connectionLine[0] - connectionPoint);

		float connectionAngle = glm::acos(glm::dot(axisDir, conLineDir));
		if (connectionAngle > glm::half_pi<float>())
			connectionAngle = glm::pi<float>() - connectionAngle;

		const float sinkAngle = glm::half_pi<float>() - connectionAngle;
		const float distance = 0.5 * m_parameters.width * (1 + glm::sin(sinkAngle));
		const glm::vec3 up(0.0, 1.0, 0.0);
		Point axisPerpDir;
		// get correct point  from perpendicular
		{
			const auto axisPerpVec = glm::cross(axisDir, up);

			Point side1 = connectionLine[1] + (axisPerpVec * m_parameters.width / 2.0f);
			Point side2 = connectionLine[1] - (axisPerpVec * m_parameters.width / 2.0f);

			if (pointsSitsOnSameHalfOfPlane(start->centre, end->centre, connectionLine[0], side1))
				axisPerpDir = glm::normalize(axisPerpVec);
			else
				axisPerpDir = -glm::normalize(axisPerpVec);
		}

		connectionPossibility.canConnect = true;
		connectionPossibility.recomendedPoint = connectionPoint + axisPerpDir * distance;;
	}

	return connectionPossibility;
}

glm::vec3 Road::getDirectionPointFromConnectionPoint(Point connectionPoint)
{
	auto& connection = findConnection(connectionPoint);

	if (m_shape.sitsOnTail(connection.point))
		return *(m_shape.getAxis().begin() + 1);
	else
		return *(m_shape.getAxis().end() - 2);
}

void Road::createPaths()
{
	float offsetPerLane = (m_parameters.laneCount / m_parameters.width) / 2.0f;
	float startOffset = 0.25f;

	// supposse we have two lanes
	m_paths.clear();
	for (int i = 0; i < m_parameters.laneCount / 2.0; ++i)
	{
		auto pathLanes = m_shape.getSkeleton();

		m_paths.insert(m_paths.begin(), std::vector(pathLanes.rbegin() + (pathLanes.size() / 2), pathLanes.rend()));
		m_paths.insert(m_paths.end(), std::vector(pathLanes.begin(), pathLanes.begin() + (pathLanes.size() / 2)));
	}
}
