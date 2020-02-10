#include "Road.h"
#include "Utilities.h"
#include "Mesh.h"
#include "LineManipulator.h"
#include "EarcutAdaptation.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>

#include <numeric>
#include <random>
#include <chrono>
#include <array>

constexpr const char* asphaltTextureLane = "resources/materials/roadTexture.jpg";

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
	auto constrainDot = [](float dotProduct)
	{
		if (dotProduct > 1.0f) return 1.0f;
		else if (dotProduct < -1.0f) return -1.0f;
	};
	float connectionAngle = glm::acos(constrainDot(glm::dot(axisDir, endToLineSegmentEnd)));

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
	return getPhysicsComponent().getCollider("BODY").collidesWith(point);
}

BasicRoad::RoadType Road::getRoadType() const
{
	return RoadType::ROAD;
}

Shape::AxisPoint Road::getAxisPoint(Point pointOnRoad) const
{
	return m_shape.getShapeAxisPoint(pointOnRoad);
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
			auto leftLineSegment = LineManipulator::getShiftedPointsFromLineToAxisInPercetageDistance(leftLanePts, axis, startOffset + offsetPerLane * i);

			Lane leftLane;
			leftLane.side = Lane::Side::LEFT;
			leftLane.points = Points(leftLineSegment.rbegin(), leftLineSegment.rend());
			leftLane.connectsTo = findConnectedRoad(axis.front());
			leftLane.connectsFrom = findConnectedRoad(axis.back());

			m_lanes[Lane::Side::LEFT].push_back(leftLane);
		}

		{
			auto rightLineSegment = LineManipulator::getShiftedPointsFromLineToAxisInPercetageDistance(rightLanePts, axis, startOffset + offsetPerLane * i);

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
			Model model;
			// basic mesh
			auto mesh = SegmentedShape::createMesh(m_shape);
			mesh.textures[VD::TextureType::DIFFUSE] = asphaltTextureLane;
			model.meshes.emplace_back(mesh);
			// line
			model.meshes.emplace_back(createLineMesh());

			Info::ModelInfo modelInfo;
			modelInfo.model = &model;

			setupModel(modelInfo, true);
		}
		//createLanes();

		// physics
		{
			auto& bodyCollider = getPhysicsComponent().createCollider("BODY");
			bodyCollider.setSelfTags({ "ROAD" });
			bodyCollider.setBoundaries(m_shape.getOutline());
			enablePhysics();
		}

		updateLength();
	}
}

void Road::reconstruct()
{
	auto axis = m_shape.getAxisPoints();

	construct(axis, m_parameters);
}

void Road::mergeWith(Road& road)
{
	m_shape.mergeWith(road.m_shape);
	road.transferConnections(this);
	reconstruct();
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

Products::Split Road::split(Shape::AxisPoint splitPoint)
{

	Products::Split product;
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
		road.construct(optSplit.value().getAxisPoints(), m_parameters);

		product.road = road;
	}
	// reconstruct t the end, may be bettter performance, who knows
	reconstruct();

	return product;
}


SegmentedShape::ShapeCut Road::getCut(Shape::AxisPoint roadAxisPoint) const
{
	return m_shape.getShapeCut(roadAxisPoint, m_width);
}

Products::Cut Road::cut(SegmentedShape::ShapeCut cutPoints)
{
	Products::Cut product;
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
		road.construct(optSplit.value().getAxisPoints(), m_parameters);
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
/*
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
*/

Mesh Road::createLineMesh()
{
	const auto axisPoints = m_shape.getAxisPoints();
	const auto leftSidePoints = m_shape.getLeftSidePoints();
	const auto rightSidePoints = m_shape.getRightSidePoints();

	const auto& lineInfo = m_parameters.lineInfo;
	// this mesh we will write on
	Mesh mesh;
	auto& meshVertices = mesh.vertices.positions;
	auto& meshIndices = mesh.indices;

	float shorterDistanceFromSide = 0;
	// statrs wiht offset of lineWidth
	float fartherDistanceFromSide = lineInfo.lineWidth;

	// apply distance from side
	shorterDistanceFromSide += lineInfo.distanceFromSide;
	fartherDistanceFromSide += lineInfo.distanceFromSide;

	// create side line
	{
		Points linePoints;
		// left side
		{
			const Points leftLineSideOne = LineManipulator::getShiftedPointsFromLineToAxisInSetDistance(axisPoints, leftSidePoints, shorterDistanceFromSide);
			const Points leftLineSideTwo = LineManipulator::getShiftedPointsFromLineToAxisInSetDistance(axisPoints, leftSidePoints, fartherDistanceFromSide);
			linePoints.insert(linePoints.end(), leftLineSideOne.begin(), leftLineSideOne.end());
			// reversed
			linePoints.insert(linePoints.end(), leftLineSideTwo.rbegin(), leftLineSideTwo.rend());
		}
		// triangulate
		const auto [leftLineVertices, leftLineIndices] = EarcutAdaptation::triangulatePoints(linePoints);
		linePoints.clear();

		// right side
		{
			const Points rightLineSideOne = LineManipulator::getShiftedPointsFromLineToAxisInSetDistance(axisPoints, rightSidePoints, shorterDistanceFromSide);
			const Points rightLineSideTwo = LineManipulator::getShiftedPointsFromLineToAxisInSetDistance(axisPoints, rightSidePoints, fartherDistanceFromSide);
			linePoints.insert(linePoints.end(), rightLineSideOne.begin(), rightLineSideOne.end());
			// reversed
			linePoints.insert(linePoints.end(), rightLineSideTwo.rbegin(), rightLineSideTwo.rend());
		}
		const auto [rightLineVertices, rightLineIndices] = EarcutAdaptation::triangulatePoints(linePoints);

		// join the
		LineManipulator::joinPositionVerticesAndIndices(meshVertices, meshIndices, leftLineVertices, leftLineIndices);
		LineManipulator::joinPositionVerticesAndIndices(meshVertices, meshIndices, rightLineVertices, rightLineIndices);
	}
	// create mid line
	if(m_parameters.laneCount > 1)
	{
		const float lineLength = 2.0f; // 2 meters
		const float spaceBetweenLines = 1.0f; //  1 meter

		auto axisPoints = m_shape.getAxisPoints();
		Point currentPoint = axisPoints.front();
		while (true)
		{
			const auto [linePoints, lineTravellLeft] = LineManipulator::cutPointsInDistance(axisPoints, lineLength);
			// if we have finished
			if (lineTravellLeft && lineTravellLeft.value() == lineLength)
				break;

			SegmentedShape shape;
			shape.construct(linePoints, m_parameters.lineInfo.lineWidth);

			const auto [lineVertices, lineIndices] = EarcutAdaptation::triangulatePoints(shape.getOutline());
			// join them
			LineManipulator::joinPositionVerticesAndIndices(meshVertices, meshIndices, lineVertices, lineIndices);

			const auto [_, stepTravellLeft] = LineManipulator::cutPointsInDistance(axisPoints, spaceBetweenLines);
			// if we have finished here as well
			if (stepTravellLeft && stepTravellLeft.value() == spaceBetweenLines)
				break;
		}
		// well cut from it
		SegmentedShape lineShape;
		lineShape.construct(m_shape.getAxis(), m_parameters.lineInfo.lineWidth);


	}

	// move vertices bit up since we want to see them above road
	for (auto& vert : meshVertices)	vert.y += 0.015f;

	// give them whiteColor
	mesh.vertices.colors = VD::ColorVertices(mesh.vertices.positions.size(), glm::vec4(1.0));

	return mesh;
}

void Road::updateWidthFromParameters()
{
	//float newWidth = 0.0f;
	// side offset
	float sideOffset = 2 * m_parameters.lineInfo.distanceFromSide + 2 * m_parameters.lineInfo.sideLineWidth;
	float totalLaneWidth = m_parameters.laneCount * m_parameters.lineInfo.laneWidth;
	float totalLineSegmentWidth = (m_parameters.laneCount - 1) * m_parameters.lineInfo.lineWidth;

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
