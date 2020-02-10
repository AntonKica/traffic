#include "RoadIntersection.h"
#include "Utilities.h"
#include "EarcutAdaptation.h"
#include "LineManipulator.h"

#include "Road.h"

#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

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

Points triangulateShape(Points shapeOutline)
{
	if (shapeOutline.size() <= 3)
		return shapeOutline;

	Points shapePoints;

	auto vertexOne = shapeOutline.begin();
	auto vertexTwo = vertexOne + 1;
	auto vertexThree = vertexTwo + 1;
	
	bool allTrianglesMade = false;
	while (!allTrianglesMade)
	{
		// make them go round
		if (vertexOne == shapeOutline.end())
			vertexOne = shapeOutline.begin();
		else if (vertexTwo == shapeOutline.end())
			vertexTwo = shapeOutline.begin();
		else if (vertexThree == shapeOutline.end())
			vertexThree = shapeOutline.begin();

		// if no more triangles left
		if (shapeOutline.size() <= 2)
		{
			allTrianglesMade = true;
		}
		else
		{
			Triangle newTriangle = { *vertexOne, *vertexTwo, *vertexThree };
			bool collapsesWithInnerWall = false;

			// basicaly the same
			LineSegment innerTriangleLine = { newTriangle[0], newTriangle[2] };
			auto linePointOne = vertexThree, linePointTwo = linePointOne + 1;

			bool allChecked = shapeOutline.size() <= 3;
			while (!allChecked)
			{
				// make them go round
				if (linePointOne == shapeOutline.end())
					linePointOne = shapeOutline.begin();
				if (linePointTwo == shapeOutline.end())
					linePointTwo = shapeOutline.begin();

				// check for end
				if (linePointOne == vertexOne)
				{
					allChecked = true;
				}
				else
				{
					LineSegment curLine = { *linePointOne, *linePointTwo };
					if (innerLineSegmentCollision(innerTriangleLine, curLine))
					{
						collapsesWithInnerWall = true;
						break;
					}

					++linePointTwo, ++linePointOne;
				}
			}
			if (!collapsesWithInnerWall)
			{
				// add current
				shapePoints.insert(shapePoints.end(), newTriangle.begin(), newTriangle.end());
				// remove edge, in this case second edge
				shapeOutline.erase(vertexTwo);
				// and reset points
				vertexOne = shapeOutline.begin();
				vertexTwo = vertexOne + 1;
				vertexThree = vertexTwo + 1;

				// dont increment
				continue;
			}
		}

		++vertexOne; ++vertexTwo; ++vertexThree;
	}

	return shapePoints;
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

/*
* circle is on their head
*
*/
void circleSortCenterOrientedSegmentedShapes(std::vector<SegmentedShape>& shapes, bool clockwise)
{
	auto getSegmentedShapeAngle = [](const SegmentedShape& shape)
	{
		const auto forward = glm::vec3(0.0, 0.0, 1.0);
		const auto up = glm::vec3(0.0, 1.0, 0.0);
		auto tailDir = glm::normalize(shape.getHead() - shape.getTail());

		float angle = glm::orientedAngle(tailDir, forward, up);
		if (angle < 0) angle += glm::two_pi<float>();

		return angle;
	};
	auto angleSegmentShapeSort = [&getSegmentedShapeAngle](const SegmentedShape& lhs, const SegmentedShape& rhs)
	{
		return getSegmentedShapeAngle(lhs) > getSegmentedShapeAngle(rhs);
	};

	if(clockwise)
		std::sort(std::begin(shapes), std::end(shapes), angleSegmentShapeSort);
	else
		std::sort(std::rbegin(shapes), std::rend(shapes), angleSegmentShapeSort);
}

std::pair<SegmentedShape, std::vector<SegmentedShape>> getMainShapeAndConnectingShapes(const std::vector<SegmentedShape>& shapes)
{
	auto firstShape = shapes.begin();
	auto secondShape = firstShape;

	float greatestAngle = -3.14;
	bool foundColinearShapes = false;
	for (auto shapeOne = shapes.begin(); shapeOne + 1 < shapes.end() && !foundColinearShapes; ++shapeOne)
	{
		auto shapeTwo = shapeOne + 1;
		while (shapeTwo != shapes.end() && !foundColinearShapes)
		{
			auto shapeOneDir = glm::normalize(shapeOne->getHead() - shapeOne->getTail());
			auto shapeTwoDir = glm::normalize(shapeTwo->getHead() - shapeTwo->getTail());

			auto dot = glm::dot(shapeOneDir, shapeTwoDir);
			// if we have found oposite vectors
			if (glm::epsilonEqual(dot, 1.0f, 0.001f))
			{
				foundColinearShapes = true;

				firstShape = shapeOne;
				secondShape = shapeTwo;
			}
			else
			{
				// if we have found oposite vectors
				float curAngle = glm::acos(glm::dot(shapeOneDir, shapeTwoDir));
				if (curAngle > greatestAngle)
				{
					greatestAngle = curAngle;

					firstShape = shapeOne;
					secondShape = shapeTwo;
				}
			}
			++shapeTwo;
		}
	}

	SegmentedShape mainShape = *firstShape;
	mainShape.mergeWith(*secondShape);

	std::vector<SegmentedShape> connectingShapes;
	for (auto begin = shapes.begin(); begin != shapes.end(); ++begin)
	{
		if (begin != firstShape && begin != secondShape)
			connectingShapes.emplace_back(*begin);
	}

	return std::make_pair(mainShape, connectingShapes);
}


void RoadIntersection::construct(std::array<Road*, 3> roads, Point intersectionPoint)
{
	m_centre = intersectionPoint;
	m_width = roads[0]->getWidth();
	m_connectShapes.clear();

	// adjust connectd roads
	{
		float requiredDistance = m_width / 2.0 + m_width * 0.25;
		for (auto& road : roads)
		{
			auto shortenShape = road->shorten(Shape::AxisPoint(intersectionPoint), requiredDistance).getShape();

			// orient to centre
			if (!shortenShape.sitsOnHead(intersectionPoint))
				shortenShape.reverseBody();

			m_connectShapes.push_back(shortenShape);

			// connect with tail
			connect(road, shortenShape.getTail());
			road->reconstruct();
		}
	}

	setUpShape();
}

void RoadIntersection::connectNewRoad(Road* newRoad)
{
	float requiredDistance = m_width / 2.0 + m_width * 0.25;
	auto shortenShape = newRoad->shorten(Shape::AxisPoint(m_centre), requiredDistance).getShape();

	// orient to centre
	if (!shortenShape.sitsOnHead(m_centre))
		shortenShape.reverseBody();

	m_connectShapes.push_back(shortenShape);

	// connect with tail
	connect(newRoad, shortenShape.getTail());
	newRoad->reconstruct();

	setUpShape();
}

bool pointIsCloserToPointsStart(const Point& point, const Points& points)
{
	if (points.size() < 2)
		return true;


	float lenghtFromStart = glm::length(point - points.front());
	float lenghtFromEnd = glm::length(point - points.back());

	return lenghtFromStart < lenghtFromEnd;
}
/*
* Edge one should be closer to side start
* from start of side to end of closerline
* and second edge goes from start if closer line to end
* to end if side
*/
/*
std::pair<Points, Points> getCutSideByTwoLinesIntoTwoEdges(Points side, Points line1, Points line2)
{
	const auto originalSide = side;
	auto [cuttedSideByLine1, _] = cutTwoTrailsOnCollision(side, line1);
	if (cuttedSideByLine1.empty())
		std::cout << "Empty\n";

	Points edgeOnePoints, edgeTwoPoints;
	if (trailTrailCollision(line2, cuttedSideByLine1))
	{
		// we cut from cutted
		auto [cuttedSideByLine2, _1] = cutTwoTrailsOnCollision(cuttedSideByLine1, line2);
		// skip common points
		// edge two
		std::copy(std::begin(side), std::end(side), std::back_inserter(edgeOnePoints));
		std::copy(std::rbegin(line1) + 1, std::rend(line1), std::back_inserter(edgeOnePoints));

		// edge one
		std::copy(std::begin(line2), std::end(line2), std::back_inserter(edgeTwoPoints));
		std::copy(std::begin(cuttedSideByLine2) + 1, std::end(cuttedSideByLine2), std::back_inserter(edgeTwoPoints));
	}
	else
	{
		// continue to we cut from side
		// ignore results
		cutTwoTrailsOnCollision(side, line2);

		// skip common points
		// edge one
		std::copy(std::begin(side), std::end(side), std::back_inserter(edgeOnePoints));
		std::copy(std::rbegin(line2) + 1, std::rend(line2), std::back_inserter(edgeOnePoints));

		// edge two
		std::copy(std::begin(line1), std::end(line1), std::back_inserter(edgeTwoPoints));
		std::copy(std::begin(cuttedSideByLine1) + 1, std::end(cuttedSideByLine1), std::back_inserter(edgeTwoPoints));
	}

	return std::make_pair(edgeOnePoints, edgeTwoPoints);
}*/

void RoadIntersection::setUpShape()
{
	// sort shapes to opposite sides
	circleSortCenterOrientedSegmentedShapes(m_connectShapes, false);

	std::vector<Points> edges;

	for (auto shapeOne = m_connectShapes.begin(), shapeTwo = shapeOne + 1;
		shapeOne != m_connectShapes.end(); ++shapeOne, ++shapeTwo)
	{
		if (shapeTwo == m_connectShapes.end())
			shapeTwo = m_connectShapes.begin();

		// get sides
		auto rightSideOne = shapeOne->getRightSidePoints();

		shapeTwo->reverseBody();
		auto rightSideTwo = shapeTwo->getRightSidePoints();
		shapeTwo->reverseBody();

		// cut them
		auto cutted = cutTwoTrailsOnCollision(rightSideOne, rightSideTwo);

		if (cutted)
		{
			// we need cut
			rightSideTwo = cutted.value().second;
			// - 1 because they share one common point
			Points edge(rightSideOne.size() + rightSideTwo.size() - 1);
			// copy them without that one common point
			auto copyIt = std::copy(rightSideOne.begin(), rightSideOne.end() - 1, edge.begin());
			std::copy(rightSideTwo.begin(), rightSideTwo.end(), copyIt);

			// add to edges
			edges.emplace_back(edge);
		}
		else
		{
			// get that point where they should intersect
			auto trailIntersectPoint = getTrailEndsIntersectionPoint(rightSideOne, rightSideTwo);

			// + 1 because we add their intersect point
			Points edge(rightSideOne.size() + rightSideTwo.size() + 1);

			// copy them with that one common point
			auto copyIt = std::copy(rightSideOne.begin(), rightSideOne.end(), edge.begin());
			*copyIt++ = trailIntersectPoint;
			std::copy(rightSideTwo.begin(), rightSideTwo.end(), copyIt);


			// add to edges
			edges.emplace_back(edge);
		}
	}

	// copy edges to outline for future use
	{
		m_outlinePoints.clear();
		for (const auto& edge : edges)
			m_outlinePoints.insert(m_outlinePoints.end(), edge.begin(), edge.end());
		// reverse since we need thme to be in clockwise order
		std::reverse(std::begin(m_outlinePoints), std::end(m_outlinePoints));

	}

	// and then triangulate
	const auto [bodyVertices, bodyIndices] =  EarcutAdaptation::triangulatePoints(m_outlinePoints);

	// then setupDrawing
	Mesh bodyMesh;
	bodyMesh.vertices.positions = bodyVertices;
	bodyMesh.indices = bodyIndices;
	const glm::vec4 grey(0.44, 0.44, 0.44, 1.0);
	bodyMesh.vertices.colors = VD::ColorVertices(bodyVertices.size(), grey);

	Mesh lineMesh = createLineMesh();


	Model model;
	model.meshes.emplace_back(bodyMesh);
	model.meshes.emplace_back(lineMesh);

	Info::ModelInfo mInfo;
	mInfo.model = &model;

	setupModel(mInfo, true);

	getPhysicsComponent().createCollider("BODY").setBoundaries(m_outlinePoints);
}

void RoadIntersection::checkShapesAndRebuildIfNeeded()
{
	// dont wipe connetions since we will needthem
	// for rebuilding former road
	// have we lost need ed connections for intersection?

	// nothing to check
	if (m_connectShapes.size() <= 2 || m_connections.size() == m_connectShapes.size())
		return;

	// check connections, which we have lost
	auto shapeIt = m_connectShapes.begin();
	while (shapeIt != m_connectShapes.end())
	{
		bool isValidShape = false;
		for (const auto& currentConnections : m_connections)
		{
			if (approxSamePoints(currentConnections.point, shapeIt->getTail()))
			{
				isValidShape = true;
				break;
			}
		}

		if (!isValidShape)
			shapeIt = m_connectShapes.erase(shapeIt);
		else
			++shapeIt;
	}

	if (m_connectShapes.size() >= 3)
	{
		setUpShape();
	}
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
	for (const auto& connectShapes : m_connectShapes)
	{
		if (connectShapes.sitsOnTail(connectionPoint))
			return *(connectShapes.getAxis().begin() + 1);
	}

	return m_centre;
}

BasicRoad::ConnectionPossibility RoadIntersection::getConnectionPossibility(LineSegment connectionLine, Shape::AxisPoint connectionPoint) const
{
	ConnectionPossibility cp{};
	if (m_connectShapes.size() < 4)
	{
		// find point which has the lowest total angle
		glm::vec3 direction;
		{
			auto dirOne = glm::normalize(*(m_connectShapes[0].getAxis().end() - 2) - m_centre);
			auto dirTwo = glm::normalize(*(m_connectShapes[1].getAxis().end() - 2) - m_centre);
			auto dirThree = glm::normalize(*(m_connectShapes[2].getAxis().end() - 2) - m_centre);

			auto angleBetweenTwoDirections = [](glm::vec3 dirOne, glm::vec3 dirTwo)
			{
				return glm::acos(glm::dot(dirOne, dirTwo));
			};

			float totalOneAngle = angleBetweenTwoDirections(dirOne, dirTwo) +
				angleBetweenTwoDirections(dirOne, dirThree);
			float totalTwoAngle = angleBetweenTwoDirections(dirTwo, dirOne) +
				angleBetweenTwoDirections(dirTwo, dirThree);
			float totalThreeAngle = angleBetweenTwoDirections(dirThree, dirOne) +
				angleBetweenTwoDirections(dirThree, dirTwo);

			float lowestAngle = std::min({ totalOneAngle, totalTwoAngle, totalThreeAngle });

			// keep in mind we nned to reverse direction
			if (totalOneAngle == lowestAngle)
				direction = -dirOne;
			else if (totalTwoAngle == lowestAngle)
				direction = -dirTwo;
			else
				direction = -dirThree;
		}

		cp.canConnect = true;
		cp.recomendedPoint = m_centre + direction * m_width / 2.0f;
	}
	else
	{
		cp.canConnect = false;
	}

	return cp;
}

void RoadIntersection::destroy()
{
}

bool RoadIntersection::hasBody() const
{
	return m_connectShapes.size() >= 3;
}

bool RoadIntersection::sitsPointOn(Point point) const
{
	return Collision::XZPointPoints(point, m_outlinePoints);
}

BasicRoad::RoadType RoadIntersection::getRoadType() const
{
	return RoadType::INTERSECTION;
}

Shape::AxisPoint RoadIntersection::getAxisPoint(Point pointOnRoad) const
{
	return Shape::AxisPoint(m_centre);
}

static Points createSubLineFromAxis(const Points& axis, const Points& line, float percentageDistanceFromAxis)
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

void RoadIntersection::createLanes()
{
	// associate road with shape
	std::vector<std::pair<SegmentedShape*, Road*>> associatedShapes;
	for (auto& shape : m_connectShapes)
	{
		auto connection = getConnection(shape.getTail());

		Road* connectedRoad = static_cast<Road*>(connection.connected);
		associatedShapes.emplace_back(std::make_pair(&shape, connectedRoad));
	}

	m_lanes.clear();
	for (const auto& [shapeOne, roadOne] : associatedShapes)
	{
		auto lanesFromRoadRoadOne = roadOne->getAllLanesConnectingTo(this);

		for (const auto& [shapeTwo, roadTwo] : associatedShapes)
		{
			// dont make road with self
			if (shapeOne == shapeTwo)
				continue;

			// create common axis and edge
			Points shapeOneAxis = shapeOne->getAxisPoints();
			Points shapeOneRightSide = shapeOne->getRightSidePoints();

			// we choose this approach since lines are cut from certain direction
			Points shapeTwoAxis = shapeTwo->getAxisPoints();
			Points shapeTwoLeftSide = shapeTwo->getLeftSidePoints();

			for (const auto& path : lanesFromRoadRoadOne)
			{
				Point intersectionLaneStart = path.points.back();
				auto shapeOneDiameter = glm::length(shapeOneAxis.front() - shapeOneRightSide.front());
				auto distFomAxisToPoint = glm::length(shapeOneAxis.front() - intersectionLaneStart);

				auto shapeOneLane = createSubLineFromAxis(shapeOneAxis, shapeOneRightSide, distFomAxisToPoint / shapeOneDiameter);
				auto shapeTwoLane = createSubLineFromAxis(shapeTwoAxis, shapeTwoLeftSide, distFomAxisToPoint / shapeOneDiameter);

				// make them interserct, but they may not intersect 
				auto optCut = cutTwoTrailsOnCollision(shapeOneLane, shapeTwoLane);
				if (!optCut)
					shapeOneLane.emplace_back(getTrailEndsIntersectionPoint(shapeOneLane, shapeTwoLane));

				// reverse second
				std::reverse(std::begin(shapeTwoLane), std::end(shapeTwoLane));	

				// make that path
				Lane newLane;
				newLane.connectsFrom = roadOne;
				newLane.connectsTo = roadTwo;

				Points pathPoints;
				std::copy(std::begin(shapeOneLane), std::end(shapeOneLane), std::back_inserter(pathPoints));
				// omit that one common point
				std::copy(std::begin(shapeTwoLane) + 1, std::end(shapeTwoLane), std::back_inserter(pathPoints));
				newLane.points = pathPoints;

				newLane.side = Lane::Side::RIGHT;

				// add that path
				m_lanes[Lane::Side::RIGHT].emplace_back(newLane);
			}
		}
	}
}

bool RoadIntersection::canSwitchLanes() const
{
	return false;
}

uint32_t RoadIntersection::directionCount() const
{
	return m_connections.size();
}

Mesh RoadIntersection::createLineMesh()
{
	Mesh mesh;
	auto& meshVertices = mesh.vertices.positions;
	auto& meshIndices = mesh.indices;

	for (uint32_t indexOne = 0, indexTwo = 1; indexOne < m_connectShapes.size(); ++indexOne, ++indexTwo)
	{
		if (indexTwo == m_connectShapes.size())
			indexTwo = 0;

		const auto& shapeOne = m_connectShapes[indexOne];
		const auto& shapeTwo = m_connectShapes[indexTwo];

		// extract points
		Points edgePoints;
		//Points axisPoints;
		{
			auto sidePointsOne =  shapeOne.getRightSidePoints();
			auto sidePointsTwo = shapeTwo.getLeftSidePoints();
			auto intersected = cutTwoTrailsOnCollision(sidePointsOne, sidePointsTwo);

			if (intersected)
				sidePointsOne.pop_back();
			else
				sidePointsOne.emplace_back(getTrailEndsIntersectionPoint(sidePointsOne, sidePointsTwo));

			edgePoints.insert(edgePoints.end(), sidePointsOne.begin(), sidePointsOne.end());
			edgePoints.insert(edgePoints.end(), sidePointsTwo.rbegin(), sidePointsTwo.rend());
		}
		
		// make them
		constexpr const auto sideDistance = RoadParameters::Defaults::distanceFromSide;
		constexpr const auto lindeWidth = RoadParameters::Defaults::lineWidth;

		const float shorterDistanceFromSide = sideDistance;
		const float fartherDistanceFromSide = sideDistance + lindeWidth;
		Points edgeLinePoints;
		{
			const Points sideOne = LineManipulator::getShiftedLineToLeftFromLineInSetDistance(edgePoints, shorterDistanceFromSide);
			const Points sideTwo = LineManipulator::getShiftedLineToLeftFromLineInSetDistance(edgePoints, fartherDistanceFromSide);
			edgeLinePoints.insert(edgeLinePoints.end(), sideOne.begin(), sideOne.end());
			// reversed
			edgeLinePoints.insert(edgeLinePoints.end(), sideTwo.rbegin(), sideTwo.rend());
		}
		// triangulate
		const auto [edgeVertices, edgeIndices] = EarcutAdaptation::triangulatePoints(edgeLinePoints);

		LineManipulator::joinPositionVerticesAndIndices(meshVertices, meshIndices, edgeVertices, edgeIndices);
	}
	// add color and move them a bit up
	for (auto& vert : meshVertices)	vert.y += 0.015f;
	// give them whiteColor
	mesh.vertices.colors = VD::ColorVertices(mesh.vertices.positions.size(), glm::vec4(1.0));

	return mesh;
}
