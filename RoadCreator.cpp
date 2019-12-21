#include "RoadCreator.h"
#include "RoadManager.h"
#include "RoadIntersection.h"

#include "GlobalObjects.h"
#include "Model.h"
#include "VulkanInfo.h"

#include <glm/gtx/perpendicular.hpp>
#include <numeric>
#include <set>

VD::PositionVertices buildSimpleShapeOutline(const std::vector<Point>& points, int width)
{
	if (points.size() < 2)
		return points;

	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	VD::PositionVertices  leftPoints;
	VD::PositionVertices  rightPoints;

	glm::vec3 dirVec;
	glm::vec3 currentDirection;
	glm::vec3 previousDirection;
	Point previousPoint;
	Point nextPoint;
	for (int i = 0; i < points.size(); ++i)
	{
		// vertices
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
	VD::PositionVertices shapePoints(leftPoints.size() + rightPoints.size());
	auto insertIt = std::copy(std::begin(leftPoints), std::end(leftPoints), shapePoints.begin());
	insertIt = std::copy(std::begin(rightPoints), std::end(rightPoints), insertIt);

	return shapePoints;
}
struct MinMaxCoords
{
	float minX = std::numeric_limits<float>::max(), maxX =  std::numeric_limits<float>::min();
	float minY = std::numeric_limits<float>::max(), maxY =  std::numeric_limits<float>::min();
	float minZ = std::numeric_limits<float>::max(), maxZ =  std::numeric_limits<float>::min();
};

MinMaxCoords getMinMaxCoordsFromPoints(const std::vector<Point>& points)
{
	MinMaxCoords minMax{};

	auto updateMinMax = [](float& min, float& max, const float& val)
	{
		min = std::min(min, val);
		max = std::max(max, val);
	};

	for (const auto& point : points)
	{
		updateMinMax(minMax.minX, minMax.maxX, point.x);
		updateMinMax(minMax.minY, minMax.maxY, point.y);
		updateMinMax(minMax.minZ, minMax.maxZ, point.z);
	}

	return minMax;
}

std::vector<Point> generateCurveFromThreePoints(const std::array<Point, 3>& points)
{
	// on same line
	if (points[0] == points[1] && points[1] == points[2])
		return std::vector(points.begin(), points.end());

	const float precision = 20;
	std::vector<Point> curvePoints(precision + 1);

	const glm::vec3 firstDir = glm::normalize(points[1] - points[0]);
	const float firstLength = glm::length(points[1] - points[0]);
	const glm::vec3 secondtDir = glm::normalize(points[2] - points[1]);
	const float secondtLenght = glm::length(points[2] - points[1]);

	for (int i = 0; i <= precision; ++i)
	{
		float distanceInPercentage = i / precision;

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

void RoadCreator::setupPrototypes()
{
	hardcodedRoadPrototypes[0] = { "Basic 2-lane road", 2, 1.0f, "resources/materials/road.png" };
	hardcodedRoadPrototypes[1] = { "Basic 4-lane road", 4, 2.0f, "resources/materials/road2.png" };
	currentPrototypeID = 0;
}
void RoadCreator::setPoint()
{
	if (validRoad && mousePoint)
		setPoints.push_back(mousePoint.value());

	createRoadIfPossible();
}

struct RoadCompare
{
	bool operator()(const Road& road1, const Road& road2) const
	{
		return std::memcmp(&road1, &road2, sizeof(Road)) == 0;
	};
};

void RoadCreator::createRoadIfPossible()
{
	// do not construct from one poin
	if (setPoints.size() <= 1 || !validRoad)
		return;
	else if ((creatorMode == Mode::STRAIGHT_LINE && setPoints.size() < 2) ||
		(creatorMode == Mode::CURVED_LINE && setPoints.size() < 3))
		return;

	Points creationPoints;
	for (const auto& sp : buildPoints)
		creationPoints.push_back(sp.point);

	// from 3 pts create something curved
	if (creatorMode == Mode::CURVED_LINE)
	{
		// tune up
		std::array<Point, 3> curvePoints;
		auto cpData = curvePoints.data();
		for (const auto& bp : buildPoints)
			if (bp.core) *(cpData++) = bp.point;

		creationPoints = generateCurveFromThreePoints(curvePoints);
		if (!buildPoints.front().core) creationPoints.insert(creationPoints.begin(), buildPoints.front().point);
		if (!buildPoints.back().core) creationPoints.insert(creationPoints.end(), buildPoints.back().point);
	}

	if (creationPoints.size())
		createRoad(creationPoints);
}

void RoadCreator::createRoad(const Points& creationPoints)
{
	const auto& currentPrototype = hardcodedRoadPrototypes[currentPrototypeID];

	std::vector<SittingPoint> constructionPoints = { buildPoints.front(), buildPoints.back() };
	if (approxSamePoints(constructionPoints[0].point, constructionPoints[1].point) || 
		(constructionPoints[0].road && constructionPoints[1].road) &&
		constructionPoints[0].road == constructionPoints[1].road)
		constructionPoints.pop_back();
	

	Road newRoad;
	newRoad.construct(creationPoints, currentPrototype.laneCount, currentPrototype.width, currentPrototype.texture);

	for (auto& constructionPoint : constructionPoints)
	{
		if (constructionPoint.road)
		{
			connectRoads(*constructionPoint.road, newRoad);

			m_pRoadManager->m_roads.remove(constructionPoint.road);
		}
	}
	m_pRoadManager->m_roads.add(newRoad);

	setPoints.clear();
}

void RoadCreator::connectRoads(const Road& road, Road& connectingRoad)
{
	auto cpAxis = connectingRoad.m_shape.getAxis();
	auto connectionsCount = connectCount(road, connectingRoad);
	if (connectionsCount == 1)
	{
		if (road.sitsOnEndPoints(cpAxis.front()) || road.sitsOnEndPoints(cpAxis.back()))
		{
			mergeRoads(connectingRoad, road);
		}
		else
		{
			buildToIntersection(road, connectingRoad);
		}
	}
	else
	{
		if (road.sitsOnEndPoints(cpAxis.front()) && road.sitsOnEndPoints(cpAxis.back()))
		{
			mergeRoads(connectingRoad, road);
		}
		else if (road.sitsOnEndPoints(cpAxis.front()) != road.sitsOnEndPoints(cpAxis.back()))
		{
			mergeRoads(connectingRoad, road);

			auto newRoad = splitKnot(connectingRoad);
			buildToIntersection(newRoad, connectingRoad);
			//m_pRoadManager->m_roads.add(newRoad);
		}
		else
		{
			buildToIntersection(road, connectingRoad);
		}
	}
}

uint32_t RoadCreator::connectCount(const Road& road, const Road& connectingRoad) const
{
	uint32_t count = 0;
	auto& shape = road.m_shape;
	auto& connectingShape = connectingRoad.m_shape;

	if (shape.sitsOnAxis(connectingShape.getTail())) ++count;
	if (shape.sitsOnAxis(connectingShape.getHead())) ++count;

	return count;
}

std::vector<Point> RoadCreator::connectPoints(const Road& road, const Road& connectingRoad) const
{
	std::vector<Point> cps;
	auto& shape = road.m_shape;
	auto& connectingShape = connectingRoad.m_shape;

	if (shape.sitsOnAxis(connectingShape.getTail())) cps.push_back(connectingShape.getTail());
	if (shape.sitsOnAxis(connectingShape.getHead())) cps.push_back(connectingShape.getHead());

	if (cps.size() > 1)
	{
		if (cps[0].x == cps[1].x && cps[0].y == cps[1].y && cps[0].z == cps[1].z)
			cps.pop_back();
	}

	return cps;
}

void RoadCreator::mergeRoads(Road& road, const Road& mergingRoad)
{
	if (road.m_parameters.width == mergingRoad.m_parameters.width)
	{
		road.mergeWith(mergingRoad);
		road.reconstruct();
	}
}

Road RoadCreator::splitKnot(Road& road)
{
	// find point which makes the knot
	Point cp {};
	auto axis = road.m_shape.getAxis();
	// do not check for same eact point
	for(int index = 0; index < axis.size(); ++index)
	{
		if (index + 2 < axis.size() &&
			pointSitsOnLine(axis[index + 1], axis[index + 2], road.m_shape.getTail()))
		{
			cp = road.m_shape.getTail();
			break;
		}
		else if (index - 1 >= 0 && 
			pointSitsOnLine(axis[index - 1], axis[index], road.m_shape.getHead()))
		{
			cp = road.m_shape.getHead();
			break;
		}
	}
	auto newShape = road.m_shape.split(cp);
	road.reconstruct();

	Road newRoad1;
	newRoad1.construct(newShape.value().getAxis(), road.m_parameters);

	return newRoad1;
}

void RoadCreator::buildToIntersection(const Road& road, Road& connectingRoad)
{
	Road splitRoad = road;
	auto connectionPoints = connectPoints(road, connectingRoad);

	//RoadIntersection intersection;
	if (connectionPoints.size() == 1)
	{
		RoadIntersection ri;
		auto optRoad = splitRoad.split(connectionPoints[0]);

		if (!optRoad)
		{
			ri.construct({ &splitRoad, &splitRoad, &connectingRoad }, connectionPoints[0]);

			new RoadIntersection(ri);
		}
		else
		{
			ri.construct({ &splitRoad, &optRoad.value(), &connectingRoad }, connectionPoints[0]);
			m_pRoadManager->m_roads.add(optRoad.value());

			new RoadIntersection(ri);
		}
	}
	else //if (connectionPoints.size() == 2)
	{
		RoadIntersection ri;
		auto optRoad = splitRoad.split(connectionPoints[0]);

		if (!optRoad)
		{
			ri.construct({ &splitRoad, &splitRoad, &connectingRoad }, connectionPoints[0]);
			new RoadIntersection(ri);

			auto newRoad = splitRoad.split(connectionPoints[1]).value();

			ri.construct({ &splitRoad, &newRoad, &connectingRoad }, connectionPoints[1]);
			new RoadIntersection(ri);

			m_pRoadManager->m_roads.add(newRoad);
		}
		else
		{
			auto newRoad1 = optRoad.value();

			ri.construct({ &splitRoad, &newRoad1, &connectingRoad }, connectionPoints[0]);
			new RoadIntersection(ri);

			// deduce which part of split is the right one
			Road* rightRoad;
			if (newRoad1.sitsOnRoad(connectingRoad.m_shape.getTail()) || 
				newRoad1.sitsOnRoad(connectingRoad.m_shape.getHead()))
			{
				rightRoad = &newRoad1;
			}
			else
			{
				rightRoad = &splitRoad;
			}

			auto newRoad2 = rightRoad->split(connectionPoints[1]).value();

			ri.construct({ &newRoad2, rightRoad, &connectingRoad }, connectionPoints[1]);
			new RoadIntersection(ri);

			// add both roads
			m_pRoadManager->m_roads.add({ newRoad1, newRoad2 });
		}
	}
	m_pRoadManager->m_roads.add(splitRoad);

}

void RoadCreator::updatePoints()
{	
	auto mousePosition = App::Scene.m_simArea.getMousePosition();
	if (mousePosition)
	{
		SittingPoint mouseSittingPoint{};
		mouseSittingPoint.core = true;

		auto selectedRoad = m_pRoadManager->getSelectedRoad();
		if (selectedRoad)
		{
			mouseSittingPoint.point = selectedRoad.value()->getPointOnRoad(mousePosition.value());
			mouseSittingPoint.road = selectedRoad.value();
		}
		else
		{
			mouseSittingPoint.point = mousePosition.value();
		}
		mousePoint = mouseSittingPoint;

		buildPoints = setPoints;
		buildPoints.push_back(mouseSittingPoint);

		// add to visalize drawing
		validRoad = true;
		if(buildPoints.size() >= 2)
		{
			std::array<Point, 2> connectPoints;
			if (buildPoints.front().road)
			{
				connectPoints = { buildPoints.begin()->point ,(buildPoints.begin() + 1)->point };
				auto recPoint = buildPoints.front().road->canConnect(connectPoints, connectPoints[0]);
				if (recPoint)
				{
					if (connectPoints[0] != recPoint)
					{
						SittingPoint sp;
						sp.point = recPoint.value();
						sp.core = true;
						buildPoints.insert(buildPoints.begin() + 1, sp);
						buildPoints.front().core = false;
					}
				}
				else 
				{
					validRoad = false;
				}
			}
			if (buildPoints.back().road)
			{
				connectPoints = { (buildPoints.end() - 1)->point ,(buildPoints.end() - 2)->point };
				auto recPoint = buildPoints.back().road->canConnect(connectPoints, connectPoints[0]);
				if (recPoint)
				{
					if (connectPoints[0] != recPoint)
					{
						SittingPoint sp;
						sp.point = recPoint.value();
						sp.core = true;
						buildPoints.insert(buildPoints.end() - 1, sp);
						buildPoints.back().core = false;
					}
				}
				else
				{
					validRoad = false;
				}
			}
		}

		Points drawPoints;
		std::transform(std::begin(buildPoints), std::end(buildPoints), std::back_inserter(drawPoints), [](const SittingPoint& sittingPoint)
			{
				return sittingPoint.point;
			});

		// cause we know we have ,mousePos
		if (creatorMode == Mode::CURVED_LINE && setPoints.size() == 2)
		{
			// tune up
			std::array<Point, 3> curvePoints;
			auto cpData = curvePoints.data();
			for (const auto& bp : buildPoints)
				if (bp.core) *(cpData++) = bp.point;

			drawPoints = generateCurveFromThreePoints(curvePoints);
			if (!buildPoints.front().core) drawPoints.insert(drawPoints.begin(), buildPoints.front().point);
			if (!buildPoints.back().core) drawPoints.insert(drawPoints.end(), buildPoints.back().point);
		}

		visualizer.setDraw(drawPoints, hardcodedRoadPrototypes[currentPrototypeID].width, validRoad);
	}
	// erase back
	else
	{
		Points pts(setPoints.size());
		std::transform(std::begin(setPoints), std::end(setPoints), std::begin(pts), [](const SittingPoint& sp) { return sp.point; });

		visualizer.setDraw(pts, hardcodedRoadPrototypes[currentPrototypeID].width, validRoad);
		mousePoint.reset();
	}
}

RoadCreator::RoadCreator(ObjectManager* roadManager)
	: m_pRoadManager(roadManager)
{
	setupPrototypes();
}

void RoadCreator::update()
{
	//here pause
	updatePoints();
	visualizer.update();
}

void RoadCreator::clickEvent()
{
	setPoint();
}

void RoadCreator::rollBackEvent()
{
	if (setPoints.size())
		setPoints.pop_back();
}


std::vector<std::string> RoadCreator::getRoadNames() const
{
	auto extractFun = [](const std::map<int, ::Prototypes>& prototypes)
	{
		std::vector<std::string> names;
		for (const auto& [id, prototype] : prototypes)
			names.push_back(prototype.name);

		return names;
	};

	static std::vector<std::string> names = extractFun(hardcodedRoadPrototypes);
	
	return names;
}

void RoadCreator::setMode(int mode)
{
	creatorMode = static_cast<Mode>(mode);
}

void RoadCreator::setPrototype(int prototype)
{
	currentPrototypeID = prototype;
}

void CreatorVisualizer::update()
{
	updateGraphics();
}

void CreatorVisualizer::setDraw(const std::vector<Point>& points, float width, bool valid)
{
	pointToDraw = points;
	this->width = width;
	this->valid = valid;
}

VD::PositionVertices CreatorVisualizer::generateLines()
{
	VD::PositionVertices points;
	for (const auto& point : pointToDraw)
		points.push_back(point);

	if(mousePoint)
		points.push_back(mousePoint.value());

	return points;
}

std::pair<VD::PositionVertices, VD::ColorVertices> CreatorVisualizer::generatePoints()
{
	auto generateCircle = []()
	{
		const int step = 45;
		VD::PositionVertices points(3* 360 / step);
		auto ptsIt = points.begin();
		for (float i = 360; i > 0; i -= step)
		{
			Point point1;
			point1.x = cos(glm::radians(i));
			point1.y = 0;
			point1.z = sin(glm::radians(i));
			point1 *= 0.08;

			Point point2;
			point2.x = cos(glm::radians(i + step));
			point2.y = 0;
			point2.z = sin(glm::radians(i + step));
			point2 *= 0.08;

			Point point3 = glm::vec3(0);

			std::array<Point, 3> pts {point1, point2, point3 };
			ptsIt = std::copy(std::begin(pts), std::end(pts), ptsIt);
		}
		return points;
	};
		

	static const auto circePoints = generateCircle();

	auto ptsToDraw = pointToDraw;
	if (mousePoint)
		ptsToDraw.push_back(mousePoint.value());

	VD::PositionVertices vertices;
	vertices.resize(ptsToDraw.size() * circePoints.size());
	auto vIter = vertices.begin();
	for (const auto point : ptsToDraw)
	{
		vIter = std::transform(std::begin(circePoints), std::end(circePoints), vIter,
			[&point](const Point& circlePoint)
			{
				return point + circlePoint;
			});
	}
	VD::ColorVertex colorVertex (0.1, 0.8, 0.1, 1.0);
	VD::ColorVertices colors(vertices.size(), colorVertex);

	return std::make_pair(vertices, colors);
}

void CreatorVisualizer::updateGraphics()
{
	if (pointToDraw.size() > 1)
	{
		Info::DrawInfo dInfo;
		dInfo.lineWidth = 2.0f;
		dInfo.polygon = VK_POLYGON_MODE_LINE;
		dInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		Mesh simpleMesh;
		VD::PositionVertices drawPoints = buildSimpleShapeOutline(pointToDraw, width);
		simpleMesh.vertices.positions = drawPoints;

		glm::vec4 color = valid ? glm::vec4(0.1, 0.2, 1.0, 1.0) : glm::vec4(1.0, 0.2, 0.1, 1.0);
		VD::ColorVertices colors(drawPoints.size(), color);
		simpleMesh.vertices.colors = colors;
		Model model;
		model.meshes.push_back(simpleMesh);

		Info::ModelInfo mInfo;
		mInfo.model = &model;

		Info::GraphicsComponentCreateInfo gInfo;
		gInfo.drawInfo = &dInfo;
		gInfo.modelInfo = &mInfo;

		lineGraphics.recreateGraphicsComponent(gInfo);
		lineGraphics.setActive(true);
	}
	else
	{
		lineGraphics.setActive(false);
	}

	const auto [vertices, colors] = generatePoints();
	if (pointToDraw.size() > 0)
	{
		Info::DrawInfo dInfo;
		dInfo.lineWidth = 1.0f;
		dInfo.polygon = VK_POLYGON_MODE_FILL;
		dInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		Mesh simpleMesh;
		simpleMesh.vertices.positions = vertices;
		simpleMesh.vertices.colors = colors;
		Model model;
		model.meshes.push_back(simpleMesh);

		Info::ModelInfo mInfo;
		mInfo.model = &model;

		Info::GraphicsComponentCreateInfo gInfo;
		gInfo.drawInfo = &dInfo;
		gInfo.modelInfo = &mInfo;

		pointGraphics.recreateGraphicsComponent(gInfo);
		pointGraphics.setActive(true);
	}
	else
	{
		pointGraphics.setActive(false);
	}
}

