#include "RoadIntersection.h"
#include "Utilities.h"
#include "polyPartition/earcut.hpp"

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
			Line innerTriangleLine = { newTriangle[0], newTriangle[2] };
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
					Line curLine = { *linePointOne, *linePointTwo };
					if (innerLineLineCollision(innerTriangleLine, curLine))
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
		return getSegmentedShapeAngle(lhs) < getSegmentedShapeAngle(rhs);
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
	m_width = roads[0]->getParameters().width;
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
		auto leftSideTwo = shapeTwo->getLeftSidePoints();

		// cut them
		bool cutted = cutTwoTrailsOnCollision(rightSideOne, leftSideTwo).has_value();

		if (cutted)
		{
			// - 1 because they share one common point
			Points edge(rightSideOne.size() + leftSideTwo.size() - 1);
			// copy them without that one common point
			auto copyIt = std::copy(rightSideOne.begin(), rightSideOne.end() - 1, edge.begin());
			std::reverse_copy(leftSideTwo.begin(), leftSideTwo.end(), copyIt);

			// add to edges
			edges.emplace_back(edge);
		}
		else
		{
			// get that point where they should intersect
			auto trailIntersectPoint = getTrailEndsIntersectionPoint(rightSideOne, leftSideTwo);

			// + 1 because we add their intersect point
			Points edge(rightSideOne.size() + leftSideTwo.size() + 1);

			// copy them with that one common point
			auto copyIt = std::copy(rightSideOne.begin(), rightSideOne.end(), edge.begin());
			*copyIt++ = trailIntersectPoint;
			std::reverse_copy(leftSideTwo.begin(), leftSideTwo.end(), copyIt);


			// add to edges
			edges.emplace_back(edge);
		}
	}

	m_outlinePoints.clear();
	// copy edges to outline
	{
		for (const auto& edge : edges)
			m_outlinePoints.insert(m_outlinePoints.end(), edge.begin(), edge.end());
	}

	// and then triangulate
	using EarCutPoint = std::array<float, 2>;
	std::vector<std::vector<EarCutPoint>> polygon(1);
	polygon[0].resize(m_outlinePoints.size());
	for (auto index = 0; index < m_outlinePoints.size(); ++index)
	{
		polygon[0][index] = { m_outlinePoints[index].x, m_outlinePoints[index].z };
	}

	std::vector<uint32_t> indices = mapbox::earcut(polygon);
	m_shapePoints = m_outlinePoints;

	// then setupDrawing
	Mesh mesh;
	mesh.vertices.positions = m_shapePoints;
	mesh.indices = indices;

	const glm::vec4 grey(0.44, 0.44, 0.44, 1.0);
	VD::ColorVertices colors(m_shapePoints.size(), grey);
	mesh.vertices.colors = colors;

	Model model;
	model.meshes.push_back(mesh);

	Info::ModelInfo mInfo;
	mInfo.model = &model;

	setupModel(mInfo, true);

	getPhysicsComponent().collider().setBoundaries(m_shapePoints);
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
	for (const auto& connectShapes : m_connectShapes)
	{
		if (connectShapes.sitsOnTail(connectionPoint))
			return *(connectShapes.getAxis().begin() + 1);
	}

	return m_centre;
}

BasicRoad::ConnectionPossibility RoadIntersection::getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const
{
	ConnectionPossibility cp{};
	if (m_connectShapes.size() < 4)
	{
		cp.canConnect = true;
		auto axis = m_connectShapes[2].getAxis();
		cp.recomendedPoint = glm::normalize(axis[1] - axis[0]) + m_centre;
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
	return m_connectShapes.size();
}

bool RoadIntersection::sitsPointOn(Point point) const
{
	return polygonPointCollision(m_outlinePoints, point);
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

void RoadIntersection::createPaths()
{
	// associate road with shape
	std::map<Road*, SegmentedShape*> associatedRoads;
	for (auto& shape : m_connectShapes)
	{
		auto connection = getConnection(shape.getTail());

		Road* connectedRoad = static_cast<Road*>(connection.connected);
		associatedRoads[connectedRoad] = &shape;
	}

	for (const auto& [roadOne, shapeOne] : associatedRoads)
	{
		auto pathsFromRoadRoadOne = roadOne->getAllPathsConnectingTo(this);

		for (const auto& [roadTwo, shapeTwo] : associatedRoads)
		{
			// dont make road with self
			if (shapeOne == shapeTwo)
				continue;

			// create common axis and edge
			Points shapeOneAxis = shapeOne->getAxisPoints();
			Points shapeTwoAxis = shapeTwo->getAxisPoints();
			std::reverse(shapeTwoAxis.begin(), shapeTwoAxis.end());

			Points shapeOneSide = shapeOne->getRightSidePoints();
			Points shapeTwoSide = shapeTwo->getLeftSidePoints();
			std::reverse(shapeTwoSide.begin(), shapeTwoSide.end());

			for (const auto& path : pathsFromRoadRoadOne)
			{
				Point intersectionPathStart = path.points.back();
				auto shapeOneDiameter = glm::length(shapeOneAxis.front() - shapeOneSide.front());
				auto distFomAxisToPoint = glm::length(shapeOneAxis.front() - intersectionPathStart);

				auto shapeOnePath = createSubLineFromAxis(shapeOneAxis, shapeOneSide, shapeOneDiameter / distFomAxisToPoint);
				auto shapeTwoPath = createSubLineFromAxis(shapeTwoAxis, shapeTwoSide, shapeOneDiameter / distFomAxisToPoint);

				// make them interserct, but they may not intersect 
				auto optCut = cutTwoTrailsOnCollision(shapeOnePath, shapeTwoPath);
				if (optCut)
					shapeTwoPath = optCut.value().second;
				else
					shapeTwoPath.emplace_back(getTrailEndsIntersectionPoint(shapeOnePath, shapeTwoPath));

				// get the cut since that we need

				// make that path
				Path newPath;
				newPath.connectsFrom = roadOne;
				newPath.connectsTo = roadTwo;

				Points pathPoints;
				std::copy(std::begin(shapeOnePath), std::end(shapeOnePath), std::back_inserter(pathPoints));
				// omit that one common point
				std::copy(std::begin(shapeTwoPath) + 1, std::end(shapeTwoPath), std::back_inserter(pathPoints));
				newPath.points = pathPoints;

				newPath.side = Path::Side::RIGHT;

				// add that path
				m_paths[Path::Side::RIGHT].emplace_back(newPath);
			}
		}
	}
}

bool RoadIntersection::canSwitchLanes() const
{
	return false;
}

void RoadIntersection::newConnecionAction()
{
}

void RoadIntersection::lostConnectionAction()
{
}

