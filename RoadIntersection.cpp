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

void RoadIntersection::construct(std::array<Road*, 3> roads, Point intersectionPoint)
{
	m_centre = intersectionPoint;
	m_width = roads[0]->getParameters().width;
	m_connectShapes.clear();

	// adjust connectd roads
	{
		float requiredDistance = m_width / 2.0f + 0.25;
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

bool pointIsCloserToPointsStart(const Point& pointsPoint, const Points& points)
{
	if (points.size() < 2)
		return true;

	float lenghtFromStart = 0, lenghtFromEnd = 0;
	bool passedPoint = false;
	for (auto p1 = points.begin(), p2 = p1 + 1; p2 != points.end(); p1 = p2++)
	{
		if (approxSamePoints(*p1, pointsPoint))
			passedPoint = true;

		// add lenght from start
		if (!passedPoint)
			lenghtFromStart += glm::length(*p1 - *p2);
		else
			lenghtFromEnd += glm::length(*p1 - *p2);
	}

	return lenghtFromStart < lenghtFromEnd;
}
/*
* Edge one should be closer to side start
* from start of side to end of closerline
* and second edge goes from start if closer line to end
* to end if side
*/
std::pair<Points, Points> getCutSideByTwoLinesIntoTwoEdges(Points side, Points line1, Points line2)
{
	const auto originalSide = side;
	auto [cuttedSideByLine1, _] = cutTwoTrailsOnCollision(side, line1);
	
	Points edgeOnePoints, edgeTwoPoints;
	if (pointIsCloserToPointsStart(line1.back(), originalSide))
	{
		// we cut from cutted
		auto [cuttedSideByLine2, _1] = cutTwoTrailsOnCollision(cuttedSideByLine1, line1);

		// skip common points
		// edge one
		std::copy(std::begin(cuttedSideByLine1), std::end(cuttedSideByLine1), std::back_inserter(edgeTwoPoints));
		std::copy(std::rbegin(line1) + 1, std::rend(line1), std::back_inserter(edgeTwoPoints));

		// edge two
		std::copy(std::begin(line2), std::end(line2), std::back_inserter(edgeOnePoints));
		std::copy(std::begin(cuttedSideByLine2) + 1, std::end(cuttedSideByLine2), std::back_inserter(edgeOnePoints));
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
}

void RoadIntersection::setUpShape()
{
	// sort them for easier manipulation
	std::vector<Points> sides;

	// sort for easier manipulation
	circleSortCenterOrientedSegmentedShapes(m_connectShapes, true);

	// this shape will be cut
	SegmentedShape mainShape = m_connectShapes[0];
	mainShape.mergeWith(m_connectShapes[2]);

	Points outlinePoints;
	// test frow mich side we cut
	{
		auto leftSide = mainShape.getLeftSidePoints();
		auto rightSide = mainShape.getRightSidePoints();

		const auto curShape = m_connectShapes[1];
		auto curLeftSide = curShape.getLeftSidePoints();
		auto curRightSide = curShape.getRightSidePoints();

		if (trailTrailCollision(curShape.getAxisPoints(), leftSide))
		{
			auto [edgeOne, edgeTwo] = getCutSideByTwoLinesIntoTwoEdges(leftSide, curLeftSide, curRightSide);

			// merge them
			std::copy(std::rbegin(rightSide), std::rend(rightSide), std::back_inserter(outlinePoints));
			std::copy(std::begin(edgeOne), std::end(edgeOne), std::back_inserter(outlinePoints));
			std::copy(std::begin(edgeTwo), std::end(edgeTwo), std::back_inserter(outlinePoints));
		}
		else
		{
			auto [edgeOne, edgeTwo] = getCutSideByTwoLinesIntoTwoEdges(rightSide, curLeftSide, curRightSide);

			// merge them
			std::copy(std::rbegin(leftSide), std::rend(leftSide), std::back_inserter(outlinePoints));
			std::copy(std::begin(edgeOne), std::end(edgeOne), std::back_inserter(outlinePoints));
			std::copy(std::begin(edgeTwo), std::end(edgeTwo), std::back_inserter(outlinePoints));
		}
	}
	// triangulate
	
	using EarCutPoint = std::array<float, 2>;
	std::vector<std::vector<EarCutPoint>> polygon(1);
	polygon[0].resize(outlinePoints.size());
	for (auto index = 0; index < outlinePoints.size(); ++index)
	{
		polygon[0][index] = { outlinePoints[index].x, outlinePoints[index].z };
	}

	std::vector<uint32_t> indices = mapbox::earcut(polygon);

	m_shapePoints = outlinePoints;

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
	return m_centre;
}

BasicRoad::ConnectionPossibility RoadIntersection::getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const
{
	ConnectionPossibility cp{};
	cp.recomendedPoint = connectionPoint;
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

void RoadIntersection::createPaths()
{
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

