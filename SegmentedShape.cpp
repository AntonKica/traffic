#include "SegmentedShape.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace
{
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

}
std::optional<SegmentedShape::Segment> SegmentedShape::selectSegment(Point arbitraryPoint) const
{
	std::optional<Segment> selectedSegment;

	for (int index = 0; index + 1 < m_joints.size(); ++index)
	{
		Points vertices = {
			m_joints[index].left, m_joints[index + 1].left,
			m_joints[index + 1].right, m_joints[index].right };

		// can help
		if (approxSamePoints(m_joints[index].centre, arbitraryPoint) || 
			approxSamePoints(m_joints[index + 1].centre, arbitraryPoint)  ||
			polygonPointCollision(vertices, arbitraryPoint))
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
			else if (index >= 1)
			{
				Segment s;
				s.start = &m_joints[index - 1];
				s.end = &m_joints[index];

				segments.emplace_back(s);
			}
			else if (index + 1 < m_joints.size())
			{
				Segment s;
				s.start = &m_joints[index];
				s.end = &m_joints[index + 1];

				segments.emplace_back(s);
			}

			break;
		}
	}
	return segments;
}

std::vector<SegmentedShape::Segment> SegmentedShape::getSegments(Shape::AxisPoint axisPoint) const
{
	std::vector<SegmentedShape::Segment> segments;

	if (sitsOnAnySegmentCorner(axisPoint))
	{
		segments = getJointSegments(axisPoint);
	}
	else
	{
		for (int index = 0; index + 1 < m_joints.size(); ++index)
		{
			if (pointSitsOnLine(m_joints[index].centre, m_joints[index + 1].centre, axisPoint))
			{
				Segment segment;
				segment.start = &m_joints[index];
				segment.end = &m_joints[index + 1];

				segments = { segment };
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

Point SegmentedShape::getCircumreferencePoint(Point shapePoint) const
{
	const auto segment = selectSegment(shapePoint).value();
	const float quarterSegmentDiameter = glm::length(segment.start->left - segment.start->right) / 4.0f;
	const float segmentLength = glm::length(segment.start->centre - segment.end->centre);

	// this sholdng happen
	const float distFromSegmentEnds = quarterSegmentDiameter > segmentLength ?
		quarterSegmentDiameter : segmentLength / 4.0f;

	// doesnt mean it is in that area...
	if (!isCirculary())
	{
		if (isTailSegment(segment))
		{
			// we need to find triangle to check
			auto startToEndDir = glm::normalize(segment.end->centre - segment.start->centre);
			auto pointToTriangle = segment.start->centre + startToEndDir * distFromSegmentEnds;

			Points vertices = { segment.start->left, segment.start->right, pointToTriangle
			};

			// if it is actualy in that area
			if (polygonPointCollision(vertices, shapePoint))
			{
				auto closestPoint = getClosestPointToLine(segment.start->left, segment.start->right, shapePoint);

				return closestPoint;
			}
		}

		if (isHeadSegment(segment))
		{
			// we need to find triangle to check
			auto endToStartDir = glm::normalize(segment.start->centre - segment.end->centre);
			auto pointToTriangle = segment.end->centre + endToStartDir * distFromSegmentEnds;

			Points vertices = { segment.end->left, segment.end->right, pointToTriangle
			};

			if (polygonPointCollision(vertices, shapePoint))
			{
				auto closestPoint = getClosestPointToLine(segment.end->left, segment.end->right, shapePoint);

				return closestPoint;
			}
		}
	}
	// .. so we have to try even for non end situation
	Line leftSide = { segment.start->left, segment.end->left };
	Line rightSide = { segment.start->right, segment.end->right };

	auto closestToLeft = getClosestPointToLine(segment.start->left, segment.end->left, shapePoint);
	auto closestToRight = getClosestPointToLine(segment.start->right, segment.end->right, shapePoint);

	auto distToLeft = glm::length(shapePoint - closestToLeft);
	auto distToRight = glm::length(shapePoint - closestToRight);

	return distToLeft < distToRight ? closestToLeft : closestToRight;
}

bool SegmentedShape::isHeadSegment(const Segment& segment) const
{
	return segment.end == &m_joints.back();
}

bool SegmentedShape::isTailSegment(const Segment& segment) const
{
	return segment.start == &m_joints.front();
}

Mesh SegmentedShape::createMesh(const SegmentedShape& shape)
{
	auto& joints = shape.getShape();
	if (joints.empty())
		return {};

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
	mesh.vertices.textures = textureCoords;
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

Points SegmentedShape::getLeftSidePoints() const
{
	Points leftSide(m_joints.size());

	{
		auto sideIter = leftSide.begin();
		for (const auto& joint : m_joints)
			*sideIter++ = joint.left;
	}
	return leftSide;
}

Points SegmentedShape::getRightSidePoints() const
{
	Points rightSide(m_joints.size());

	{
		auto sideIter = rightSide.begin();
		for (const auto& joint : m_joints)
			*sideIter++ = joint.right;
	}

	return rightSide;
}

Shape::Axis SegmentedShape::getAxis() const
{
	Shape::Axis axis(m_joints.size());

	auto axisIter = axis.begin();
	for (const auto& joint : m_joints)
		*axisIter++ = joint.centre;

	return axis;
}

Points SegmentedShape::getAxisPoints() const
{
	Points axis(m_joints.size());

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
	if (hasTailDirectionPoint())
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
	if (hasHeadDirectionPoint())
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
	cp.points = constructionPoints;

	construct(cp, width);
}

void SegmentedShape::construct(const OrientedConstructionPoints& constructionPoints, float width)
{
	clearDirectionPoints();

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

		Shape::Axis::iterator splitIter;
		if (sitsOnAnySegmentCorner(splitPoint))
			splitIter = std::find(std::begin(newAxis), std::end(newAxis), splitPoint);
		else
			splitIter = insertElemementBetween(newAxis, firstPoint, secondPoint, splitPoint);
		//dont erase if same 
		// recteate this worldAxis
		Shape::Axis curSplitAxis(std::begin(newAxis), splitIter + 1);
		Shape::Axis secondSplitAxis(splitIter, std::end(newAxis));


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
			if (!sitsOnAnySegmentCorner(splitPoint))
				insertElemementBetween(newAxis, firstPoint->centre, secondPoint->centre, splitPoint);

			setNewCirclePointsStart(newAxis, splitPoint);
			construct(newAxis, m_width);
		}
	}

	return optionalSplit;
}

SegmentedShape SegmentedShape::cutKnot()
{
	auto knotPoint = sitsOnAxis(getHead()) ? getHead() : getTail();

	auto newAxis = getAxis();

	const auto [firstPoint, secondPoint] = getEdgesOfAxisPoint(knotPoint);
	auto knotIter = insertElemementBetween(newAxis, firstPoint, secondPoint, knotPoint);

	// get the right points
	Points knotPoints;
	if (sitsOnHead(knotPoint))
	{
		std::copy(newAxis.begin(), knotIter + 1, std::back_inserter(knotPoints));
		newAxis.erase(newAxis.begin(), knotIter);
	}
	else
	{
		std::copy(knotIter, newAxis.end(), std::back_inserter(knotPoints));
		newAxis.erase(knotIter + 1, newAxis.end());
	}

	// construct them
	construct(newAxis, m_width);

	SegmentedShape knotShape;
	knotShape.construct(knotPoints, m_width);

	return knotShape;
}

SegmentedShape SegmentedShape::shorten(Shape::AxisPoint shapeEnd, float size)
{
	Shape::Axis newAxis;
	Shape::Axis shortenAxis;
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

			// place behind
			auto insertIt = axis.emplace(firstPoint + 1, Shape::AxisPoint(pointPos));

			// copy shortened
			shortenAxis = Shape::Axis(axis.begin(), insertIt + 1);
			// then rese
			newAxis = Shape::Axis(insertIt, axis.end());
		}
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

			// place behind
			auto insertIt = reversedAxis.emplace(firstPoint + 1, Shape::AxisPoint(pointPos));

			// reverse copy shortened
			std::reverse_copy(std::begin(reversedAxis), insertIt + 1, std::back_insert_iterator(shortenAxis));
			// then rest
			std::reverse_copy(insertIt, std::end(reversedAxis), std::back_insert_iterator(newAxis));
		}
	}
	// update current
	construct(newAxis, m_width);

	// create shortened
	SegmentedShape shortenedShape;
	shortenedShape.construct(shortenAxis, m_width);

	return shortenedShape;
}

void SegmentedShape::extend(Shape::AxisPoint shapeEnd, Point point)
{
	auto newAxis = getAxis();
	if (shapeEnd == newAxis.front())
		newAxis.emplace(newAxis.begin(), point);
	else if (shapeEnd == newAxis.back())
		newAxis.emplace(newAxis.end(), point);
	else
		throw std::runtime_error("Unknown extent point!");

	construct(newAxis, m_width);
}

SegmentedShape::ShapeCut SegmentedShape::getShapeCut(Shape::AxisPoint axisPoint, float radius) const
{
	auto axis = getAxis();

	// insert if unique
	if (std::find(std::begin(axis), std::end(axis), axisPoint) == std::end(axis))
	{
		auto [firstPoint, secondPoint] = getEdgesOfAxisPoint(axisPoint);
		insertElemementBetween(axis, firstPoint, secondPoint, axisPoint);
	}
	if (isCirculary())
	{
		setNewCirclePointsStart(axis, axisPoint);
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
	else if (remainingFromEnd)
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
	if (std::equal(std::begin(axis), std::end(axis), std::begin(cutPoints.axis), std::end(cutPoints.axis)))
	{
		*this = {};
	}
	else
	{
		// sits on edns so we want to preserve current but always cut from back
		if (sitsOnTail(cutPoints.axis.front()))
		{
			auto cut = split(cutPoints.axis.back()).value();

			construct(cut.getAxis(), m_width);
		}
		else if (sitsOnHead(cutPoints.axis.back()))
		{
			// just split, let cut out
			split(cutPoints.axis.front());

			// but dont construct
		}
		else
		{
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
				auto [firstEdge, secondEdge] = getEdgesOfAxisPoint(cutPoints.axis.front());
				auto insertIt = insertElemementBetween(axis, firstEdge, secondEdge, cutPoints.axis.front());
				setNewCirclePointsStart(axis, *insertIt);

				for (uint32_t indexOne = 0, indexTwo = 1; indexTwo < axis.size(); ++indexOne, ++indexTwo)
				{
					if (pointSitsOnLine(axis[indexOne], axis[indexTwo], cutPoints.axis.back()))
					{
						insertIt = insertElemementBetween(axis, axis[indexOne], axis[indexTwo], cutPoints.axis.back());
						axis.erase(axis.begin(), insertIt);

						break;
					}
				}
				construct(axis, m_width);
				/*else
				{
					throw std::runtime_error("Not yet implemented circular cut");
				}*/
			}
		}
	}

	return optShape;
}

void SegmentedShape::setNewCircularEndPoints(Shape::AxisPoint axisPoint)
{
	auto newAxis = getAxis();
	setNewCirclePointsStart(newAxis, axisPoint);

	construct(newAxis, m_width);
}

/*
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
}*/

void SegmentedShape::purifyPoints(Points& axis)
{
	if (axis.size() < 3)
		return;
	// remove duplicates, but not end points, if theyre equal
	auto cursor = axis.begin();
	while (cursor + 1 != axis.end())
	{
		auto compare = cursor + 1;
		while (compare != axis.end())
		{
			// comparing ends
			if (cursor == axis.begin() && compare == axis.end() - 1)
				break; //we can safely

			if (approxSamePoints(*cursor, *compare))
			{
				compare = axis.erase(compare);
			}
			else
			{
				++compare;
			}
		}
		++cursor;
	}

	// check for colinear lines
	auto pointOne = axis.begin();
	auto pointTwo = pointOne + 1;
	auto pointThree = pointTwo + 1;

	while (true)
	{
		const auto firstDir = glm::normalize(*pointTwo - *pointOne);
		const auto secondDir = glm::normalize(*pointThree - *pointTwo);

		const glm::vec3 front(0.0f, 0.0f, 1.0f);
		auto firstAngle = glm::acos(glm::dot(firstDir, front));
		auto secondAngle = glm::acos(glm::dot(secondDir, front));

		// remove that poing
		if (glm::epsilonEqual(firstAngle, secondAngle, 0.0001f))
			axis.erase(pointTwo);

		// move points
		++pointOne;
		pointTwo = pointOne + 1;
		if (pointTwo != axis.end())
			pointThree = pointTwo + 1;

		// check if we exceded the rande
		if (pointTwo == axis.end() || pointThree == axis.end())
			break;
	}
}

void SegmentedShape::createShape(Points axis)
{
	// remove duplicates and colinear lines
	purifyPoints(axis);

	// construct
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