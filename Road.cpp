#include "Road.h"
#include "Utilities.h"
#include "Mesh.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>

#include <numeric>
#include <random>
#include <chrono>
#include <array>

constexpr const char* asphaltTextureLane = "resources/materials/road1.jpg";

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

BasicRoad::ConnectionPossibility Road::getConnectionPossibility(LineSegment connectionLineSegment, Shape::AxisPoint connectionPoint) const
{
	ConnectionPossibility connectionPossibility{};
	connectionPossibility.canConnect = true;
	connectionPossibility.recomendedPoint = connectionPoint;
	//reored 
	if (approxSamePoints(connectionLineSegment[0], connectionPoint))
		std::swap(connectionLineSegment[0], connectionLineSegment[1]);

	// sits on ceonneted end?
	if (m_shape.sitsOnTailOrHead(connectionPoint))
	{
		for (const auto& connection : m_connections)
		{
			if (approxSamePoints(connection.point, connectionPoint))
				connectionPossibility.canConnect = false;
		}
	}

	// line[1] is on rectangle axis
	auto getRectangleLineSegmentouchPoint =
		[](LineSegment rectangleAxis, float rectangleWidth, LineSegment line, bool useIntersectionOffset)
	{
		const glm::vec3 axisDir = glm::normalize(rectangleAxis[1] - rectangleAxis[0]);
		const glm::vec3 conLineSegmentDir = glm::normalize(line[0] - line[1]);

		float connectionAngle = glm::acos(glm::dot(axisDir, conLineSegmentDir));
		if (connectionAngle > glm::half_pi<float>())
			connectionAngle = glm::pi<float>() - connectionAngle;

		const float sinkAngle = glm::half_pi<float>() - connectionAngle;
		const float distance = 0.5 * rectangleWidth * (1 + glm::sin(sinkAngle));
		const glm::vec3 up(0.0, 1.0, 0.0);
		Point axisPerpDir;
		// get correct point  from perpendicular
		{
			const auto axisPerpVec = glm::cross(axisDir, up);

			Point side1 = line[1] + (axisPerpVec * rectangleWidth / 2.0f);
			Point side2 = line[1] - (axisPerpVec * rectangleWidth / 2.0f);

			if (pointsSitsOnSameHalfOfPlane(rectangleAxis[0], rectangleAxis[1], line[0], side1))
				axisPerpDir = glm::normalize(axisPerpVec);
			else
				axisPerpDir = -glm::normalize(axisPerpVec);
		}
		// just for now
		const float harcodedIntersectionOffset = useIntersectionOffset ? 0.25f : 0.0f;

		return line[1] + axisPerpDir * (distance + harcodedIntersectionOffset);
	};

	auto lineSegment = getClosestLineSegmentFromPoint(m_shape.getAxisPoints(), connectionPoint);

	// ends on ends
	if (lineSegment[0] != connectionPoint)
		std::swap(lineSegment[0], lineSegment[1]);

	glm::vec3 axisDir = glm::normalize(lineSegment[0] - lineSegment[1]);
	glm::vec3 endToLineSegmentEnd = glm::normalize(connectionLineSegment[0] - connectionLineSegment[1]);
	float connectionAngle = glm::acos(glm::dot(axisDir, endToLineSegmentEnd));

	// I really dont know how to write thi in one satement
	if (!m_shape.sitsOnTailOrHead(connectionPoint))
	{
		connectionPossibility.recomendedPoint =
			getRectangleLineSegmentouchPoint(lineSegment, m_width, connectionLineSegment, true);
	}
	else if (connectionAngle > glm::half_pi<float>())
	{
		connectionPossibility.recomendedPoint =
			getRectangleLineSegmentouchPoint(lineSegment, m_width, connectionLineSegment, false);
	}
	// sits on corner?
	/*
	if (segments.size() == 1)
	{
		auto& [start, end] = segments;
		// ends on ends
		if (end->centre != connectionPoint)
			std::swap(start, end);

		glm::vec3 axisDir = glm::normalize(end->centre - start->centre);
		glm::vec3 endToLineSegmentEnd = glm::normalize(connectionLineSegment[0] - connectionLineSegment[1]);
		float connectionAngle = glm::acos(glm::dot(axisDir, endToLineSegmentEnd));

		// I really dont know how to write thi in one satement
		if (!m_shape.sitsOnTailOrHead(connectionPoint))
		{
			connectionPossibility.recomendedPoint =
				getRectangleLineSegmentouchPoint(LineSegment{ start->centre, end->centre }, m_width, connectionLineSegment, true);
		}
		else if (connectionAngle > glm::half_pi<float>())
		{
			connectionPossibility.recomendedPoint =
				getRectangleLineSegmentouchPoint(LineSegment{ start->centre, end->centre }, m_width, connectionLineSegment, false);
		}
	}
	else if (segments.size() == 2)// sits between corners/ joints
	{
		const auto& [start1, end1] = segments[0];
		const auto& [start2, end2] = segments[1];

		const glm::vec3 axisDir1 = glm::normalize(end1->centre - start1->centre);
		// since end1 == start 2
		const glm::vec3 axisDir2 = glm::normalize(start2->centre - end2->centre);
		const glm::vec3 lineEndStart = glm::normalize(connectionLineSegment[0] - connectionLineSegment[1]);

		const auto angleArea = glm::angle(axisDir1, axisDir2);
		const auto angleLineSegmentAxis1 = glm::angle(axisDir1, lineEndStart);
		const auto angleLineSegmentAxis2 = glm::angle(axisDir2, lineEndStart);

		// for now, if between sharp angles,its not valid
		connectionPossibility.canConnect = angleLineSegmentAxis1 <= angleArea && angleLineSegmentAxis2 <= angleArea;
	}
	else
	{
		throw std::runtime_error("Bad segment point: " + glm::to_string(Point(connectionPoint)) + " !");
	}
	*/

	return connectionPossibility;
}

glm::vec3 Road::getDirectionPointFromConnectionPoint(Point connectionPoint)
{
	auto& connection = getConnection(connectionPoint);

	if (m_shape.sitsOnTail(connection.point))
		return *(m_shape.getAxis().begin() + 1);
	else
		return *(m_shape.getAxis().end() - 2);
}

void Road::destroy()
{
}

bool Road::hasBody() const
{
	return m_shape.getAxis().size();
}

bool Road::sitsPointOn(Point point) const
{
	//return m_shape.sitsOnShape(point);
	return getPhysicsComponent().collider().collides(point);
}

BasicRoad::RoadType Road::getRoadType() const
{
	return RoadType::ROAD;
}

Shape::AxisPoint Road::getAxisPoint(Point pointOnRoad) const
{
	return m_shape.getShapeAxisPoint(pointOnRoad);
}

static Points createSubLineSegmentFromAxis(const Points& axis, const Points& line, float percentageDistanceFromAxis)
{
	Points newPoints(line);
	for (uint32_t index = 0; index < axis.size(); ++index)
	{
		const auto dir = glm::normalize(line[index] - axis[index]);
		const auto dist = glm::length(line[index] - axis[index]);
		newPoints[index] = axis[index] + (dir * dist * percentageDistanceFromAxis);
	}

	return newPoints;
}

void Road::createLanes()
{
	// half width
	float offsetPerLane = 1.0 / m_parameters.laneCount;
	float startOffset = offsetPerLane;

	auto sortToLeftAndRightPoints = [](const Points& pts)
	{
		Points leftPts(pts.size() / 2);
		auto leftIt = leftPts.begin();
		Points rightPts(pts.size() / 2);
		auto rightIt = rightPts.begin();

		auto ptsIt = pts.begin();
		while (ptsIt != pts.end())
		{
			*leftIt++ = *ptsIt++;
			*rightIt++ = *ptsIt++;
		}

		return std::make_pair(leftPts, rightPts);
	};

	const auto [leftLanePts, rightLanePts] = sortToLeftAndRightPoints(m_shape.getSkeleton());

	// transform to normal points
	Points axis = m_shape.getAxisPoints();

	// supposse we have two lanes
	m_lanes.clear();
	for (int i = 0; i < m_parameters.laneCount / 2; ++i)
	{
		{
			auto leftLineSegment = createSubLineSegmentFromAxis(leftLanePts, axis, startOffset + offsetPerLane * i);

			Lane leftLane;
			leftLane.side = Lane::Side::LEFT;
			leftLane.points = Points(leftLineSegment.rbegin(), leftLineSegment.rend());
			leftLane.connectsTo = findConnectedRoad(axis.front());
			leftLane.connectsFrom = findConnectedRoad(axis.back());

			m_lanes[Lane::Side::LEFT].push_back(leftLane);
		}

		{
			auto rightLineSegment = createSubLineSegmentFromAxis(rightLanePts, axis, startOffset + offsetPerLane * i);

			Lane rightLane;
			rightLane.side = Lane::Side::RIGHT;
			rightLane.points = Points(rightLineSegment.begin(), rightLineSegment.end());
			rightLane.connectsTo = findConnectedRoad(axis.back());
			rightLane.connectsFrom = findConnectedRoad(axis.front());

			m_lanes[Lane::Side::RIGHT].push_back(rightLane);
		}
	}
}

bool Road::canSwitchLanes() const
{
	// for now
	return true;
}

RoadParameters::Parameters Road::getParameters() const
{
	return m_parameters;
}

float Road::getWidth() const
{
	return m_width;
}

float Road::getLength() const
{
	return m_length;
}

bool Road::sitsOnEndPoints(const Point& point) const
{
	return m_shape.sitsOnTailOrHead(point);
}

Shape::Axis Road::getAxis() const
{
	return m_shape.getAxis();
}

Points Road::getAxisPoints() const
{
	return m_shape.getAxisPoints();
}

Point Road::getCircumreferencePoint(Point roadPoint) const
{
	return m_shape.getCircumreferencePoint(roadPoint);
}

const SegmentedShape& Road::getShape() const
{
	return m_shape;
}

Shape::AxisPoint Road::getClosestEndPoint(Shape::AxisPoint axisPoint) const
{
	auto axisPoints = m_shape.getAxisPoints();

	float lengthFromStart = 0;
	float totalLength = 0;
	float passedPoint = false;
	for (uint32_t firstIndex = 0, secondIndex = 1; secondIndex < axisPoints.size(); ++firstIndex, ++secondIndex)
	{
		const LineSegment curLineSegment = { axisPoints[firstIndex], axisPoints[secondIndex] };
		if (pointSitsOnLineSegment(axisPoint, curLineSegment) && !passedPoint)
		{
			passedPoint = true;

			lengthFromStart += glm::length(curLineSegment[0] - axisPoint);
			totalLength += glm::length(curLineSegment[0] - curLineSegment[1]);
		}
		else
		{
			if (!passedPoint)
				lengthFromStart += glm::length(curLineSegment[0] - curLineSegment[1]);

			totalLength += glm::length(curLineSegment[0] - curLineSegment[1]);
		}
	}
	auto lengthFromEnd = totalLength - lengthFromStart;

	return lengthFromStart < lengthFromEnd ? m_shape.getTail() : m_shape.getHead();
}

glm::vec3 Road::getDirectionFromEndPoint(Shape::AxisPoint endPoint) const
{
	auto axis = m_shape.getAxisPoints();

	if (m_shape.sitsOnTail(endPoint))
	{
		return glm::normalize(m_shape.getTail() - *(axis.begin() - 1));
	}
	else
	{
		return glm::normalize(m_shape.getHead() - *(axis.end() - 2));
	}
}


void Road::construct(Shape::Axis axisPoints, const RoadParameters::Parameters& parameters)
{
	Points points(axisPoints.begin(), axisPoints.end());

	construct(points, parameters);
}

void Road::construct(Points creationPoints, const RoadParameters::Parameters& parameters)
{
	if (creationPoints.empty())
	{
		m_shape = SegmentedShape();
		m_parameters = {};
		m_width = 0;
		m_length = 0;
	}
	else
	{
		m_parameters = parameters;
		updateWidthFromParameters();

		SegmentedShape::OrientedConstructionPoints cps;
		cps.points = creationPoints;
		for (const auto& cp : m_connections)
		{
			if (approxSamePoints(creationPoints.front(), cp.point))
				cps.tailDirectionPoint = cp.connected->getDirectionPointFromConnectionPoint(creationPoints.front());
			else
				cps.headDirectionPoint = cp.connected->getDirectionPointFromConnectionPoint(creationPoints.back());
		}
		m_shape.construct(cps, m_width);

		// graphics model
		{
			auto mesh = SegmentedShape::createMesh(m_shape);
			mesh.textures[VD::TextureType::DIFFUSE] = asphaltTextureLane;

			Model model;
			model.meshes.push_back(mesh);

			Info::ModelInfo modelInfo;
			modelInfo.model = &model;

			setupModel(modelInfo, true);
		}
		//createLanes();

		// physics
		{
			auto& physicComponent = getPhysicsComponent();
			physicComponent.collider().setBoundaries(m_shape.getOutline());
			enablePhysics();

			physicComponent.setSelfCollisionTags({ "ROAD" });
		}

		updateLength();
	}
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
		for (auto& connection : m_connections)
		{
			if (!sitsOnEndPoints(connection.point))
			{
				road.addConnection(connection);
				//product.connection = connection;
				dismissConnection(connection);
			}
		}
		// then construct
		road.construct(optSplit.value().getAxis(), m_parameters);

		product.road = road;
	}
	// reconstruct t the end, may be bettter performance, who knows
	reconstruct();

	return product;
}

Road Road::cutKnot()
{
	auto knot = m_shape.cutKnot();

	Road newRoad;
	// transfer connections if any, we dont care atm
	transferConnections(&newRoad);

	// construct 'em
	reconstruct();
	newRoad.construct(knot.getAxisPoints(), m_parameters);

	return newRoad;
}


Road Road::shorten(Shape::AxisPoint roadEnd, float size)
{
	auto shortShape = m_shape.shorten(roadEnd, size);
	reconstruct();

	Road newRoad;
	newRoad.construct(shortShape.getAxisPoints(), m_parameters);
	return newRoad;
}

void Road::extend(Shape::AxisPoint roadEnd, Point point)
{
	m_shape.extend(roadEnd, point);
	reconstruct();
}

SegmentedShape::ShapeCut Road::getCut(Shape::AxisPoint roadAxisPoint) const
{
	return m_shape.getShapeCut(roadAxisPoint, m_width);
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
				road.addConnection(connection);
				dismissConnection(connection);
			}
		}
		// then construct
		road.construct(optSplit.value().getAxis(), m_parameters);
		reconstruct();

		product.road = road;

	}
	else if (m_shape.getAxis().size()) // purge connections
	{
		for (auto& connection : m_connections)
		{
			if (!sitsOnEndPoints(connection.point))
				dismissConnection(connection);
		}

		reconstruct();

	}

	return product;
}

void Road::resetNearbyBuildings()
{
	m_nearbyBuildings.clear();
}

void Road::addNearbyByuilding(BasicBuilding* nearbyBuilding, Point entryPoint)
{
	NearbyBuildingPlacement placement;
	placement.building = nearbyBuilding;
	placement.entryPoint = entryPoint;

	m_nearbyBuildings.emplace_back(placement);
}

const std::vector<Road::NearbyBuildingPlacement>& Road::getNearbyBuildings() const
{
	return m_nearbyBuildings;
}

void Road::updateWidthFromParameters()
{
	//float newWidth = 0.0f;
	// side offset
	float sideOffset = 2 * m_parameters.distanceFromSide + 2 * m_parameters.sideLineWidth;
	float totalLaneWidth = m_parameters.laneCount * m_parameters.laneWidth;
	float totalLineSegmentWidth = (m_parameters.laneCount - 1) * m_parameters.lineWidth;

	m_width = sideOffset + totalLaneWidth + totalLineSegmentWidth;
}

void Road::updateLength()
{
	m_length = 0.0f;
	auto axisPoints = m_shape.getAxisPoints();

	if (axisPoints.size() >= 2)
	{
		for (auto pointOne = axisPoints.begin(), pointTwo = axisPoints.begin() + 1;
			pointTwo != axisPoints.end(); ++pointOne, ++pointTwo)
		{
			m_length += glm::length(*pointOne - *pointTwo);
		}

	}
}
