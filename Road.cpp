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

std::optional<SegmentedShape::Segment> SegmentedShape::selectSegment(const Point& point) const
{
	std::optional<Segment> selectedSegment;

	for (int index = 0; index + 1 < m_joints.size(); ++index)
	{
		Points vertices = {
			m_joints[index].side.left, m_joints[index + 1].side.left,
			m_joints[index + 1].side.right, m_joints[index].side.right };

		if (polygonPointCollision(vertices, point))
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

std::vector<SegmentedShape::Segment> SegmentedShape::getJointSegments(const Point& jointPoint) const
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

std::optional<Point> SegmentedShape::getShapeAxisPoint(const Point& point) const
{
	std::optional<Point> axisPoint;

	auto selectedSegment = selectSegment(point);
	if (selectedSegment)
	{
		const auto& [start, end] = selectedSegment.value();
		Point shapePoint = getClosestPointToLine(start->centre, end->centre, point);

		// dont oveerlap actual shape and snap to joint
		{
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
			axisPoint = glm::length(head - point) < glm::length(tail - point) ? head : tail;
		else
			axisPoint = shapePoint;
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

		*verticesIter++ = joint.side.left;
		*verticesIter++ = joint.side.right;

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
		*skeletonIter++ = joint.side.left;
		*skeletonIter++ = joint.side.right;
	}

	return skeleton;
}

Points SegmentedShape::getAxis() const
{
	Points axis(m_joints.size());

	auto axisIter = axis.begin();
	for (const auto& joint : m_joints)
		*axisIter++ = joint.centre;

	return axis;
}

Point SegmentedShape::getHead() const
{
	return m_joints.back().centre;
}

Point SegmentedShape::getTail() const
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
	for (int index = 0; index + 1 < m_joints.size(); ++index)
	{
		Points vertices = {
			m_joints[index].side.left, m_joints[index + 1].side.left, 
			m_joints[index + 1].side.right, m_joints[index].side.right};
		if (polygonPointCollision(vertices, point))
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
		std::swap(joint.side.left, joint.side.right);
}

void SegmentedShape::construct(const Points& axis, float width)
{
	m_width = width;
	createShape(axis);
}

void SegmentedShape::megeWith(const SegmentedShape& otherShape)
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
		construct(thisShapeAxis, m_width);
	}
	else
	{
		std::copy(std::begin(otherShapeAxis) + 1, std::end(otherShapeAxis), std::back_inserter(thisShapeAxis));
		construct(thisShapeAxis, m_width);
	}
}

std::optional<SegmentedShape> SegmentedShape::split(const Point& splitPoint)
{
	std::optional<SegmentedShape> optionalSplit;

	if (!sitsOnTailOrHead(splitPoint) && !isCirculary())
	{
		Points newAxis = getAxis();
		const auto [firstPoint, secondPoint] = selectSegment(splitPoint).value();

		auto newSplitIter = insertElemementBetween(newAxis, firstPoint->centre, secondPoint->centre, splitPoint);
		//dont erase if same 
		// recteate this worldAxis
		Points curSplitAxis (std::begin(newAxis), newSplitIter + 1);
		Points secondSplitAxis(newSplitIter, std::end(newAxis));


		construct(curSplitAxis, m_width);

		SegmentedShape optShape;
		optShape.construct(secondSplitAxis, m_width);
		optionalSplit = optShape;
	}
	else if (isCirculary())
	{
		// move axis to splitPoints
		Points newAxis = getAxis();

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

Point SegmentedShape::shorten(const Point& shapeEnd, float size)
{
	Points newAxis;
	Point shortenPosition = {};
	if (sitsOnTail(shapeEnd))
	{
		Points axis = getAxis();
		auto [firstPoint, secondPoint, travelledDistance] = travellDistanceOnPoints(axis, size);
		if (secondPoint == std::end(axis))
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
		axis.erase(axis.begin(), firstPoint);
		newAxis = axis;
	}
	else
	{
		reverseBody();
		//reversed 
		Points reversedAxis = getAxis();
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
		auto copyIt = std::reverse_copy(firstPoint, std::end(reversedAxis), std::back_insert_iterator(newAxis));
	}

	construct(newAxis, m_width);
	return shortenPosition;
}

void SegmentedShape::setNewCircularEndPoints(const Point& point)
{
	auto newAxiss = getAxis();
	eraseCommonPoints();
	auto newAxis = getAxis();
	auto pointIt = std::find(std::begin(newAxis), std::end(newAxis), point);// findJoint(m_joints, point);
	{
		int elementsToErase = pointIt - newAxis.begin();
		newAxis.insert(newAxis.end(), newAxis.begin(), pointIt);
		newAxis.erase(newAxis.begin(), newAxis.begin() + elementsToErase);

		/// add same point to other end
		newAxis.insert(newAxis.end(), newAxis.front());
	}

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

			Joint newJoint;
			newJoint.centre = curPoint;
			newJoint.side = { left, right };

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

Point Road::getPointOnRoad(const Point& point)
{
	return m_shape.getShapeAxisPoint(point).value();
}

void Road::construct(Points axisPoints, uint32_t laneCount, float width, std::string texture)
{
	m_position = {};

	m_parameters.laneCount = laneCount;
	m_parameters.width = width;
	m_parameters.texture = texture;
	m_shape.construct(axisPoints, width);

	// model
	auto mesh = SegmentedShape::createMesh(m_shape);
	mesh.textures[VD::TextureType::DIFFUSE] = texture;

	Mesh sh;
	sh.vertices.positions = m_shape.getAxis();
	Model model;
	model.meshes.push_back(mesh);
	//model.meshes[0] = sh;

	Info::ModelInfo modelInfo;
	modelInfo.model = &model;

	setupModel(modelInfo, true);
	createPaths();
}

void Road::reconstruct()
{
	auto mesh = SegmentedShape::createMesh(m_shape);
	mesh.textures[VD::TextureType::DIFFUSE] = m_parameters.texture;

	Mesh sh;
	sh.vertices.positions = m_shape.getAxis();
	Model model;
	model.meshes.push_back(mesh);
	//model.meshes[0] = sh;

	Info::ModelInfo modelInfo;
	modelInfo.model = &model;

	setupModel(modelInfo, true);
	createPaths();
}

void Road::mergeWithRoad(const Road& road)
{
	m_shape.megeWith(road.m_shape);
	reconstruct();
}

std::optional<Road> Road::split(const Point& splitPoint)
{
	auto optSplit = m_shape.split(splitPoint);
	reconstruct();

	std::optional<Road> optRoad;
	if (optSplit)
	{
		Road road;
		road.construct(optSplit.value().getAxis(), m_parameters.laneCount, m_parameters.width, m_parameters.texture);

		optRoad = road;
	}

	return optRoad;
}


Point Road::shorten(const Point& roadEnd, float size)
{
	auto shortPoint = m_shape.shorten(roadEnd, size);
	reconstruct();
	return shortPoint;
}

std::optional<Point> Road::canConnect(std::array<Point, 2> connectionLine, const Point& connectionPoint) const
{
	//reored 
	if (approxSamePoints(connectionLine[0], connectionPoint))
		std::swap(connectionLine[0], connectionLine[1]);

	std::optional<Point> recommendedPoint;
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
				recommendedPoint = connectionPoint;
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

			if(angleLineS1 > roadAngle || angleLineS2 > roadAngle)
				recommendedPoint = connectionPoint;
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

		const float sinkAngle = glm::half_pi<float>()- connectionAngle;
		const float distance = 0.5 * m_parameters.width * (1 +  glm::sin(sinkAngle));
		std::cout << distance << '\n';
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

		recommendedPoint = connectionPoint + axisPerpDir * distance;
	}

	return recommendedPoint;
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
