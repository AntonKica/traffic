#include "RoadCreator.h"
#include "RoadIntersection.h"

#include "Collisions.h"
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

RoadCreatorUI::RoadCreatorUI()
{
	m_prototypes = { 
	{ "Basic 2-lane road", 2, 1.0f, "resources/materials/road.png" },
	{ "Basic 4-lane road", 4, 2.0f, "resources/materials/road2.png" } ,
	{ "Basic 2-lane road", 2, 1.0f, "resources/materials/road.png" },
	{ "Basic 4-lane road", 4, 2.0f, "resources/materials/road2.png" },
	{ "Basic 2-lane road", 2, 1.0f, "resources/materials/road.png" },
	{ "Basic 4-lane road", 4, 2.0f, "resources/materials/road2.png" },
	{ "Basic 2-lane road", 2, 1.0f, "resources/materials/road.png" },
	};

	m_selectedPrototype = &m_prototypes[0];
}

void RoadCreatorUI::draw()
{
	// ImVec2(150, 200))
	ImGui::Columns(2, 0, false);
	ImGui::SetColumnWidth(0, 120.f);
	ImGui::Checkbox("Curved lines", &m_doCurves);
	ImGui::NextColumn();

	ImGui::BeginChild(" roads ");
	//
	constexpr const uint32_t maxColumnsPerLine = 4;
	const uint32_t nOfRows = m_prototypes.size() / maxColumnsPerLine + 1;
	
	for (uint32_t row = 0; row < nOfRows; ++row)
	{
		ImGui::Columns(maxColumnsPerLine, 0, false);
		const uint32_t totalIterations = m_prototypes.size();
		const uint32_t nOfColumns = (row + 1) * maxColumnsPerLine <= totalIterations ?
			maxColumnsPerLine : totalIterations - row * maxColumnsPerLine;
		for (uint32_t column = 0; column < nOfColumns; ++column)
		{
			const uint32_t currentIndex = row * maxColumnsPerLine + column;
			const ImVec2 buttonSize(ImGui::GetColumnWidth() * 0.9f, 40.0f);

			const auto& currentPrototype = m_prototypes[currentIndex];

			if (ImGui::Button(currentPrototype.name.c_str(), buttonSize))
				m_selectedPrototype = &currentPrototype;

			ImGui::NextColumn();
		}
	}

	ImGui::EndChild();
	ImGui::NextColumn();
}

bool RoadCreatorUI::doCurves() const
{
	return m_doCurves;
}

const RC::Prototypes* RoadCreatorUI::getSelectedPrototype() const
{
	return m_selectedPrototype;
}

RC::ProcSitPts RoadCreator::processSittingPoints(const std::vector<RC::SittingPoint> sittingPoints) const
{
	RC::ProcSitPts processed{};
	if (sittingPoints.size() == 1)
	{
		processed.axisPoints = { sittingPoints[0].point };
	}
	else if(sittingPoints.size() > 1)
	{
		// extract points
		for (const auto& sp : sittingPoints)
			processed.axisPoints.push_back(sp.point);

		// process ends
		const auto& front = sittingPoints.front();
		const auto& back = sittingPoints.back();

		if (front.road)
		{
			processed.roadBeginAxisPoint = std::make_pair(front.point, front.road.value());

			Line connectingLine = { front.point, (sittingPoints.begin() + 1)->point };
			auto conPossibiliy = front.road.value()->getConnectionPossibility(connectingLine, Shape::AxisPoint(front.point));

			processed.validPoints = conPossibiliy.canConnect;
			processed.axisPoints.insert(processed.axisPoints.begin() + 1, conPossibiliy.recomendedPoint);
		}
		if (back.road)
		{
			processed.roadEndAxisPoint = std::make_pair(back.point, back.road.value());

			Line connectingLine = { back.point, (sittingPoints.end() - 2)->point };
			auto conPossibiliy = back.road.value()->getConnectionPossibility(connectingLine, Shape::AxisPoint(back.point));

			processed.validPoints = conPossibiliy.canConnect;
			processed.axisPoints.insert(processed.axisPoints.end() - 1, conPossibiliy.recomendedPoint);
		}
	}

	return processed;
}

void RoadCreator::setPoint()
{
	if (m_processedCurrentPoints.validPoints && m_currentShapeValid && m_mousePoint)
	{
		m_setPoints.push_back(m_mousePoint.value());

		handleCurrentPoints();
	}
}

void RoadCreator::validateCurrentShape()
{
	// doesntqoek rhe best
	m_currentShapeValid = true;
	if (m_currentMode == Creator::CreatorMode::DESTROY)
		return;

	if (m_creationPoints.points.size() >= 2)
	{
		Points shapePoints = m_creationPoints.points;
		auto shapeOutline = buildSimpleShapeOutline(shapePoints, m_ui.getSelectedPrototype()->width);

		removeDuplicates(shapeOutline);
		Points leftPts(std::begin(shapeOutline), std::begin(shapeOutline) + std::size(shapeOutline) / 2);
		Points rightPts(std::begin(shapeOutline) + std::size(shapeOutline) / 2, std::end(shapeOutline));

		// remove poin ts sitting on road ends
		if (m_processedCurrentPoints.roadBeginAxisPoint)
		{
			const auto& road = *m_processedCurrentPoints.roadBeginAxisPoint.value().second;
			/*if (!leftPts.empty() && road.sitsPointOn(*leftPts.begin()))
				leftPts.erase(leftPts.begin());
			if (!rightPts.empty() && road.sitsPointOn(*rightPts.begin()))
				rightPts.erase(rightPts.begin());*/

			bool removePoints = (!leftPts.empty() && road.sitsPointOn(*leftPts.begin())) ||
				!rightPts.empty() && road.sitsPointOn(*rightPts.begin());

			if (removePoints)
			{
				if(leftPts.size())
					leftPts.erase(leftPts.begin());
				if (rightPts.size())
					rightPts.erase(rightPts.begin());
			}
		}
		if (m_processedCurrentPoints.roadEndAxisPoint)
		{
			const auto& road = *m_processedCurrentPoints.roadEndAxisPoint.value().second;

			/*if (!leftPts.empty() && road.sitsPointOn(*(leftPts.end() - 1)))
				leftPts.erase(leftPts.end() - 1);
			if (!rightPts.empty() && road.sitsPointOn(*(rightPts.end() - 1)))
				rightPts.erase(rightPts.end() - 1);*/

			bool removePoints = (!leftPts.empty() && road.sitsPointOn(*(leftPts.end() - 1))) ||
				!rightPts.empty() && road.sitsPointOn(*(rightPts.end() - 1));

			if (removePoints)
			{
				if (leftPts.size())
					leftPts.erase(leftPts.end() - 1);
				if (rightPts.size())
					rightPts.erase(rightPts.end() - 1);
			}
		}

		auto joinLeftAndRightPoints = [](const Points& leftPts, const Points& rightPts)
		{
			Points joinedPts(leftPts.size() + rightPts.size());
			auto dataIt = joinedPts.begin();
			// Were doing this this way becaouse they may be one more point on the
			// left side and on the right side as well
			auto leftIter = leftPts.begin();
			auto rightIter = rightPts.begin();
			while (true)
			{
				if (leftIter != leftPts.end())
					*(dataIt++) = *(leftIter++);
				if (rightIter != rightPts.end())
					*(dataIt++) = *(rightIter++);

				if (leftIter == leftPts.end() && rightIter == rightPts.end())
					break;
			}

			return joinedPts;
		};
		Collider2D col (joinLeftAndRightPoints(leftPts, rightPts));


		for (const auto& road : m_pObjectManager->m_roads.data)
		{
			//auto skeleton = road.m_shape.getSkeleton();

			if (col.collides(road.collider2D))
			{
				m_currentShapeValid = false;
				break;
			}
		}
	}
}

void RoadCreator::handleCurrentPoints()
{
	if (m_currentMode == Creator::CreatorMode::CREATE)
		tryToConstructRoad();
	else
		tryToDestroyRoad();
}

void RoadCreator::tryToConstructRoad()
{
	if (!m_processedCurrentPoints.validPoints)
		return;
	if (m_setPoints.size() <= 1)
		return;
	if (m_setPoints.size() <= 2 && m_ui.doCurves())
		return;

	createRoadFromCurrent();
}

void RoadCreator::tryToDestroyRoad()
{
	if (m_setPoints.empty())
		return;

	auto& curSetPoint = m_setPoints[0];
	if (curSetPoint.road)
	{
		auto& curPoint = curSetPoint.point;

		auto& curRoad = curSetPoint.road.value();

		if (curRoad->getRoadType() == BasicRoad::RoadType::ROAD)
		{
			auto road = dynamic_cast<Road*>(curRoad);
			auto cut = road->getCut(Shape::AxisPoint(curPoint));
			auto product = road->cut(cut);

			if (!road->hasBody())
				m_pObjectManager->m_roads.remove(road);

			if (product.road)
			{
				m_pObjectManager->m_roads.add(product.road.value());
				/*if (product.connection)
					addedRoad->addConnection(product.connection.value());*/
			}

			tidyIntersections();
		}
		else
		{
			auto intersection = dynamic_cast<RoadIntersection*>(curRoad);
			m_pObjectManager->m_intersections.remove(intersection);
		}

	}

	m_setPoints.clear();
}

void RoadCreator::tidyIntersections()
{
	auto& intersections = m_pObjectManager->m_intersections.data;
	for (auto intiter = intersections.begin(); intiter != intersections.end();)
	{
		if (!intiter->validIntersection())
		{
			auto dissassembledRoads = intiter->disassemble();
			if (dissassembledRoads.size() == 2)
			{
				dissassembledRoads[0]->mergeWith(*dissassembledRoads[1]);
				m_pObjectManager->m_roads.remove(dissassembledRoads[1]);
			}

			intiter = m_pObjectManager->m_intersections.remove(&*intiter);
		}
		else
		{
			++intiter;
		}
	}
}

void RoadCreator::createRoadFromCurrent()
{
	std::vector<RC::PointRoadPair> connectingPoints;
	if (m_processedCurrentPoints.roadBeginAxisPoint)
		connectingPoints.push_back(m_processedCurrentPoints.roadBeginAxisPoint.value());
	if (m_processedCurrentPoints.roadEndAxisPoint)
		connectingPoints.push_back(m_processedCurrentPoints.roadEndAxisPoint.value());
	if (connectingPoints.size() == 2 && connectingPoints[0].second == connectingPoints[1].second)
		connectingPoints.pop_back();


	// creater road
	const auto& currentPrototype = *m_ui.getSelectedPrototype();

	Road newRoad;
	newRoad.construct(m_creationPoints.points, currentPrototype.laneCount, currentPrototype.width, currentPrototype.texture);

	handleConstruction(newRoad, connectingPoints);

	m_setPoints.clear();
}

void RoadCreator::handleConstruction(Road newRoad, std::vector<RC::PointRoadPair> connectPoints)
{

	// max two points
	std::vector<RoadIntersection> newIntersections;
	std::vector<Road> newRoads;

	bool addNewRoad = true;
	Road* useRoad = &newRoad;
	for (auto& [point, bRoad] : connectPoints)
	{
		if (bRoad->getRoadType() == BasicRoad::RoadType::ROAD)
		{
			auto road = dynamic_cast<Road*>(bRoad);

			auto connectProducts = connectRoads(*road, *useRoad);

			newIntersections.insert(newIntersections.begin(),
				connectProducts.newIntersections.begin(), connectProducts.newIntersections.end());
			newRoads.insert(newRoads.begin(),
				connectProducts.newRoads.begin(), connectProducts.newRoads.end());

			// use road from current connecting
			if (!connectProducts.keepConnectingRoad && addNewRoad)
			{
				useRoad = road;
				addNewRoad = false;
			}
			// iterated second time , if merged we have to destroy used road
			else if (useRoad != &newRoad && !connectProducts.keepConnectingRoad)
			{
				m_pObjectManager->m_roads.remove(useRoad);
				useRoad = nullptr;
			}
		}
	}

	if (addNewRoad)
		newRoads.emplace_back(newRoad);
	// add to map or wherever
	m_pObjectManager->m_roads.add(newRoads);
	m_pObjectManager->m_intersections.add(newIntersections);

}
/*
*	Updates evry point creator has
*/
void RoadCreator::updatePoints()
{
	updateMousePoint();
	updateCurrentPoints();
	updateCreationPoints();

	// befpre set
	validateCurrentShape();

	setVisualizerDraw();
}

void RoadCreator::updateMousePoint()
{
	// only if theres mouse
	if (auto mousePosition = App::Scene.m_simArea.getMousePosition())
	{
		const auto& mousePoint = mousePosition.value();
		RC::SittingPoint newSittingPoint;

		//just for now this way of getting selected road
		if (auto selectedRoad = m_pObjectManager->getSelectedRoad())
		{
			const auto& road = selectedRoad.value();
			newSittingPoint.road = selectedRoad.value();
			if (road->getRoadType() == BasicRoad::RoadType::ROAD)
			{
				auto roadPoint = selectedRoad.value()->getAxisPoint(mousePoint);

				newSittingPoint.point = roadPoint;
			}
		}
		else
		{
			newSittingPoint.point = mousePoint;
		}

		// and asign
		m_mousePoint = newSittingPoint;
	}
	else
	{
		m_mousePoint.reset();
	}
}

void RoadCreator::updateCurrentPoints()
{
	if (m_currentMode == Creator::CreatorMode::CREATE)
	{
		RC::SittingPoints spts = m_setPoints;
		if (m_mousePoint)
			spts.push_back(m_mousePoint.value());

		m_processedCurrentPoints = processSittingPoints(spts);
	}
	else
	{
		m_processedCurrentPoints = {};
		if (m_mousePoint)
		{
			const auto& mp = m_mousePoint.value();
			if (mp.road)
			{
				const auto& bRoad = mp.road.value();
				if (bRoad->getRoadType() == BasicRoad::RoadType::ROAD)
				{
					auto road = dynamic_cast<Road*>(bRoad);
					// get cut
					auto cutPts = road->getCut(Shape::AxisPoint(mp.point)).axis;

					// and transfer to pCPts
					m_processedCurrentPoints.axisPoints.resize(cutPts.size());
					auto cpIt = m_processedCurrentPoints.axisPoints.begin();
					for (const auto& pt : cutPts)
						*(cpIt++) = pt;
				}
				else
				{
					auto road = dynamic_cast<RoadIntersection*>(bRoad);

					m_processedCurrentPoints.axisPoints = { road->getAxisPoint(mp.point) };
				}
			}
		}
	}
}

void RoadCreator::updateCreationPoints()
{
	Points creationPoints;
	bool hasFrontRoad = false;
	bool hasBackRoad = false;

	if (m_ui.doCurves() && m_setPoints.size() == 2)
	{
		const auto& curPts = m_processedCurrentPoints.axisPoints;
		std::array<Point, 3> curvePoints;
		if (hasFrontRoad = m_processedCurrentPoints.roadBeginAxisPoint.has_value())
		{
			creationPoints.push_back(curPts[0]);
			curvePoints[0] = curPts[1];
			curvePoints[1] = curPts[2];
		}
		else
		{
			curvePoints[0] = curPts[0];
			curvePoints[1] = curPts[1];
		}
		if (hasBackRoad = m_processedCurrentPoints.roadEndAxisPoint.has_value())
		{
			creationPoints.push_back(curPts[curPts.size() - 1]);
			curvePoints[2] = curPts[curPts.size() - 2];
		}
		else
		{
			curvePoints[2] = curPts[curPts.size() - 1];
		}

		auto curve = generateCurveFromThreePoints(curvePoints);
		// insert with offset
		if (m_processedCurrentPoints.roadBeginAxisPoint)
			creationPoints.insert(creationPoints.begin() + 1, curve.begin(), curve.end());
		else
			creationPoints.insert(creationPoints.begin(), curve.begin(), curve.end());
	}
	else
	{
		hasFrontRoad = m_processedCurrentPoints.roadBeginAxisPoint.has_value();
		hasBackRoad = m_processedCurrentPoints.roadEndAxisPoint.has_value();
		creationPoints = m_processedCurrentPoints.axisPoints;
	}

	m_creationPoints.points = creationPoints;
	m_creationPoints.frontOnRoad = hasFrontRoad;
	m_creationPoints.backOnRoad = hasBackRoad;

	removeDuplicates(m_creationPoints.points);
}

void RoadCreator::setVisualizerDraw()
{
	visualizer.setDraw(m_creationPoints.points, m_processedCurrentPoints.axisPoints,
		m_ui.getSelectedPrototype()->width, m_processedCurrentPoints.validPoints && m_currentShapeValid);
}

/*
struct RoadCompare
{
	bool operator()(const Road& road1, const Road& road2) const
	{
		return std::memcmp(&road1, &road2, sizeof(Road)) == 0;
	};
};*/

RoadCreator::ConnectProducts RoadCreator::connectRoads(Road& road, Road& connectingRoad)
{
	RoadCreator::ConnectProducts connectProducts;
	connectProducts.keepConnectingRoad = true;

	auto cpAxis = connectingRoad.m_shape.getAxis();
	auto connectionsCount = connectCount(road, connectingRoad);
	if (connectionsCount == 1)
	{
		if (road.sitsOnEndPoints(cpAxis.front()) || road.sitsOnEndPoints(cpAxis.back()))
		{
			mergeRoads(road, connectingRoad);
			connectProducts.keepConnectingRoad = false;
		}
		else
		{
			const auto [newRoads, newInterSections] = buildToIntersection(road, connectingRoad);
			connectProducts.newRoads = newRoads;
			connectProducts.newIntersections = newInterSections;
		}
	}
	else
	{
		if (road.sitsOnEndPoints(cpAxis.front()) && road.sitsOnEndPoints(cpAxis.back()))
		{
			mergeRoads(road, connectingRoad);
			connectProducts.keepConnectingRoad = false;
		}
		else if (road.sitsOnEndPoints(cpAxis.front()) != road.sitsOnEndPoints(cpAxis.back()))
		{
			mergeRoads(road, connectingRoad);
			connectProducts.keepConnectingRoad = false;

			auto cut = cutKnot(road);
			const auto [newRoads, newInterSections] = buildToIntersection(road, cut);

			connectProducts.newRoads = newRoads;
			connectProducts.newRoads.push_back(cut);
			connectProducts.newIntersections = newInterSections;
		}
		else
		{
			const auto [newRoads, newInterSections] = buildToIntersection(road, connectingRoad);
			connectProducts.newRoads = newRoads;
			connectProducts.newIntersections = newInterSections;
		}
	}

	return connectProducts;
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

std::vector<Shape::AxisPoint> RoadCreator::connectPoints(const Road& road, const Road& connectingRoad) const
{
	std::vector<Shape::AxisPoint> cps;
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

void RoadCreator::mergeRoads(Road& road, Road& mergingRoad)
{
	if (road.m_parameters.width == mergingRoad.m_parameters.width)
	{
		road.mergeWith(mergingRoad);
		road.reconstruct();
	}
}

Road RoadCreator::cutKnot(Road& road)
{
	// find point which makes the knot
	Shape::AxisPoint cp{};
	auto axis = road.m_shape.getAxis();
	// do not check for same eact point
	for (int index = 0; index < axis.size(); ++index)
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
	//auto [optRoad, optConnection] = road.split(cp);
	auto knot = road.split(cp).road.value();
	// swap since cut is now in road shape
	if (knot.m_shape.isCirculary())
	{
		std::swap(knot.m_shape, road.m_shape);
		road.transferConnections(&knot);
	}

	road.reconstruct();
	knot.reconstruct();

	return knot;
}

RoadCreator::IntersectionProducts RoadCreator::buildToIntersection(Road& road, Road& connectingRoad)
{
	RoadCreator::IntersectionProducts products;

	auto connectionPoints = connectPoints(road, connectingRoad);

	//RoadIntersection intersection;
	if (connectionPoints.size() == 1)
	{
		RoadIntersection intersection;
		auto product = road.split(connectionPoints[0]);

		if (!product.road)
		{
			intersection.construct({ &road,&road, &connectingRoad }, connectionPoints[0]);

			products.newIntersections = { intersection };
		}
		else
		{
			// add right awaY
			auto newRoad = product.road.value();
			intersection.construct({ &road, &newRoad, &connectingRoad }, connectionPoints[0]);

			products.newRoads = { newRoad };
			products.newIntersections = { intersection };
		}
	}
	else //if (connectionPoints.size() == 2)
	{
		//RoadIntersection* intersection = new RoadIntersection;
		//auto [optRoad, optConnection] = road->split(connectionPoints[0]);
		auto optRoad = road.split(connectionPoints[0]).road;

		if (!optRoad)
		{
			RoadIntersection firstIntersection;
			firstIntersection.construct({ &road, &road, &connectingRoad }, connectionPoints[0]);

			auto secondProduct = road.split(connectionPoints[1]);
			auto newRoad = secondProduct.road.value();
			//if (optConnection)
			//	addedRoad->addConnection(optConnection.value());

			RoadIntersection secondIntersection;
			secondIntersection.construct({ &road, &newRoad, &connectingRoad }, connectionPoints[1]);

			products.newRoads = { newRoad };
			products.newIntersections = { firstIntersection, secondIntersection };
		}
		else
		{
			auto firstNewRoad = optRoad.value();

			RoadIntersection firstIntersection;
			firstIntersection.construct({ &road, &firstNewRoad, &connectingRoad }, connectionPoints[0]);

			// deduce which part of split is the right one
			Road* rightRoad;
			if (firstNewRoad.sitsPointOn(connectingRoad.m_shape.getTail()) ||
				firstNewRoad.sitsPointOn(connectingRoad.m_shape.getHead()))
			{
				rightRoad = &firstNewRoad;
			}
			else
			{
				rightRoad = &road;
			}

			//auto [optRoad2, optConnection2] = rightRoad->split(connectionPoints[1]);
			auto secondNewRoad = rightRoad->split(connectionPoints[1]).road.value();
			// add right away as well
			//if (optConnection2)
			//	addedRoad2->addConnection(optConnection2.value());

			RoadIntersection secondIntersection;
			secondIntersection.construct({ &secondNewRoad, rightRoad, &connectingRoad }, connectionPoints[1]);

			products.newRoads = { firstNewRoad, secondNewRoad };
			products.newIntersections = { firstIntersection, secondIntersection };
		}
	}

	return products;
}

void RoadCreator::setCreatorModeAction()
{
	// could be clar points function tho
	m_setPoints.clear();
	m_creationPoints = {};

	visualizer.setActive(m_active);
}

void RoadCreator::setActiveAction()
{
	m_setPoints.clear();
	m_creationPoints = {};

	visualizer.setActive(m_active);
}

RoadCreator::RoadCreator(ObjectManager* objectManager)
	: BasicCreator(objectManager)
{
}

void RoadCreator::update()
{
	if (m_active)
	{
		updatePoints();

		visualizer.update();
	}
}

void RoadCreator::clickEvent()
{
	if (!m_active)
		return;

	setPoint();
}

void RoadCreator::rollBackEvent()
{
	if (m_setPoints.size())
		m_setPoints.pop_back();
}

void CreatorVisualizer::update()
{
	updateGraphics();
}

void CreatorVisualizer::setDraw(const std::vector<Point>& drawAxis, const std::vector<Point>& drawPoints, float width, bool valid)
{
	axisToDraw = drawAxis;
	pointToDraw = drawPoints;
	this->width = width;
	this->valid = valid;
}

void CreatorVisualizer::setActive(bool active)
{
	pointGraphics.setActive(active);
	lineGraphics.setActive(active);
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
			point1.y = 0.015;
			point1.z = sin(glm::radians(i));
			point1 *= 0.08;

			Point point2;
			point2.x = cos(glm::radians(i + step));
			point2.y = 0.015;
			point2.z = sin(glm::radians(i + step));
			point2 *= 0.08;

			Point point3 = glm::vec3(0.0, 0.015, 0.0);

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
	if (axisToDraw.size() > 1)
	{
		Info::DrawInfo dInfo;
		dInfo.lineWidth = 2.0f;
		dInfo.polygon = VK_POLYGON_MODE_LINE;
		dInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		Mesh simpleMesh;
		VD::PositionVertices drawPoints = buildSimpleShapeOutline(axisToDraw, width);
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
