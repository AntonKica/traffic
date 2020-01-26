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

BasicRoad::ConnectionPossibility Road::getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const
{
	ConnectionPossibility connectionPossibility{};
	connectionPossibility.canConnect = true;
	connectionPossibility.recomendedPoint = connectionPoint;
	//reored 
	if (approxSamePoints(connectionLine[0], connectionPoint))
		std::swap(connectionLine[0], connectionLine[1]);

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
	auto getRectangleLineouchPoint =
		[](Line rectangleAxis, float rectangleWidth, Line line, bool useIntersectionOffset)
	{
		const glm::vec3 axisDir = glm::normalize(rectangleAxis[1] - rectangleAxis[0]);
		const glm::vec3 conLineDir = glm::normalize(line[0] - line[1]);

		float connectionAngle = glm::acos(glm::dot(axisDir, conLineDir));
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

	auto segments = m_shape.getSegments(connectionPoint);
	// sits on corner?
	if (segments.size() == 1)
	{
		auto& [start, end] = segments[0];
		// ends on ends
		if (end->centre != connectionPoint)
			std::swap(start, end);

		glm::vec3 axisDir = glm::normalize(end->centre - start->centre);
		glm::vec3 endToLineEnd = glm::normalize(connectionLine[0] - connectionLine[1]);
		float connectionAngle = glm::acos(glm::dot(axisDir, endToLineEnd));

		// I really dont know how to write thi in one satement
		if (!m_shape.sitsOnTailOrHead(connectionPoint))
		{
			connectionPossibility.recomendedPoint =
				getRectangleLineouchPoint(Line{ start->centre, end->centre }, m_parameters.width, connectionLine, true);
		}
		else if (connectionAngle > glm::half_pi<float>())
		{
			connectionPossibility.recomendedPoint =
				getRectangleLineouchPoint(Line{ start->centre, end->centre }, m_parameters.width, connectionLine, false);
		}
	}
	else if (segments.size() == 2)// sits between corners/ joints
	{
		const auto& [start1, end1] = segments[0];
		const auto& [start2, end2] = segments[1];

		const glm::vec3 axisDir1 = glm::normalize(end1->centre - start1->centre);
		// since end1 == start 2
		const glm::vec3 axisDir2 = glm::normalize(start2->centre - end2->centre);
		const glm::vec3 lineEndStart = glm::normalize(connectionLine[0] - connectionLine[1]);

		const auto angleArea = glm::angle(axisDir1, axisDir2);
		const auto angleLineAxis1 = glm::angle(axisDir1, lineEndStart);
		const auto angleLineAxis2 = glm::angle(axisDir2, lineEndStart);

		// for now, if between sharp angles,its not valid
		connectionPossibility.canConnect = angleLineAxis1 <= angleArea && angleLineAxis2 <= angleArea;
	}
	else
	{
		throw std::runtime_error("Bad segment point: " + glm::to_string(Point(connectionPoint)) + " !");
	}

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
	return m_shape.sitsOnShape(point);
}

BasicRoad::RoadType Road::getRoadType() const
{
	return RoadType::ROAD;
}

Shape::AxisPoint Road::getAxisPoint(Point pointOnRoad) const
{
	return m_shape.getShapeAxisPoint(pointOnRoad).value();
}

Points createSubLineFromAxis(const Points& axis, const Points& line, float percentageDistanceFromAxis)
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

void Road::createPaths()
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
	m_paths.clear();
	for (int i = 0; i < m_parameters.laneCount / 2; ++i)
	{
		{
			auto leftLine = createSubLineFromAxis(leftLanePts, axis, startOffset + offsetPerLane * i);

			Path leftPath;
			leftPath.side = Path::Side::LEFT;
			leftPath.points = Points(leftLine.rbegin(), leftLine.rend());
			leftPath.connectsTo = findConnectedRoad(axis.front());
			leftPath.connectsFrom = findConnectedRoad(axis.back());

			m_paths[Path::Side::LEFT].push_back(leftPath);
		}

		{
			auto rightLine = createSubLineFromAxis(rightLanePts, axis, startOffset + offsetPerLane * i);

			Path rightPath;
			rightPath.side = Path::Side::RIGHT;
			rightPath.points = Points(rightLine.begin(), rightLine.end());
			rightPath.connectsTo = findConnectedRoad(axis.back());
			rightPath.connectsFrom = findConnectedRoad(axis.front());

			m_paths[Path::Side::RIGHT].push_back(rightPath);
		}
	}
}

bool Road::canSwitchLanes() const
{
	// for now
	return true;
}

Road::RoadParameters Road::getParameters() const
{
	return m_parameters;
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

SegmentedShape Road::getShape() const
{
	return m_shape;
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
	if (creationPoints.empty())
	{
		m_shape = SegmentedShape();
	}
	else
	{
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

		// graphics model
		{
			auto mesh = SegmentedShape::createMesh(m_shape);
			mesh.textures[VD::TextureType::DIFFUSE] = m_parameters.texture;

			Model model;
			model.meshes.push_back(mesh);

			Info::ModelInfo modelInfo;
			modelInfo.model = &model;

			setupModel(modelInfo, true);
		}
		//createPaths();

		// physics
		{
			auto& physicComponent = getPhysicsComponent();
			physicComponent.setActive(true);
			auto pts = m_shape.getSkeleton();
			physicComponent.collider().setBoundaries(Points(pts.begin(), pts.end()));

			Info::PhysicsComponentUpdateTags updateTags;
			updateTags.newTags = { "ROAD" };
			updateTags.newOtherTags = {};
			physicComponent.updateCollisionTags(updateTags);
		}
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
		for(auto& connection : m_connections)
		{
			if (!sitsOnEndPoints(connection.point))
			{
				road.addConnection(connection);
				//product.connection = connection;
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
				road.addConnection(connection);
				dismissConnection(connection);
			}
		}
		// then construct
		road.construct(optSplit.value().getAxis(), m_parameters.laneCount, m_parameters.width, m_parameters.texture);
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

void Road::newConnecionAction()
{
	reconstruct();
}

void Road::lostConnectionAction()
{
	reconstruct();
}
