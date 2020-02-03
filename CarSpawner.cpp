#include "CarSpawner.h"
#include "Road.h"
#include <numeric>
#include <random>
#include <chrono>
#include "SimpleCar.h"

void CarSpawner::update()
{
	if (m_disabled)
		return;

}

glm::vec3 CarSpawner::getDirectionPointFromConnectionPoint(Point connectionPoint)
{
	return getPosition();
}

BasicRoad::ConnectionPossibility CarSpawner::getConnectionPossibility(Line connectionLine, Shape::AxisPoint connectionPoint) const
{
	return ConnectionPossibility();
}

void CarSpawner::destroy()
{
}

bool CarSpawner::hasBody() const
{
	return !m_connections.empty();
}

bool CarSpawner::sitsPointOn(Point point) const
{
	Points pts(m_shapePoints.size());
	std::transform(std::begin(m_shapePoints), std::end(m_shapePoints), std::begin(pts),
		[&](const Point& point)
		{
			return point + getPosition();
		});

	return polygonPointCollision(pts, point);
}

BasicRoad::RoadType CarSpawner::getRoadType() const
{
	return RoadType::CAR_SPAWNER;
}

Shape::AxisPoint CarSpawner::getAxisPoint(Point pointOnRoad) const
{
	return Shape::AxisPoint();
}

static Points getMovedPointsInDirectionAndDistance(Points points, glm::vec3 direction, float distance)
{
	for (auto& point : points)
		point += direction * distance;

	return points;
}

void CarSpawner::createLanes()
{
	// we know theres one
	auto& connectedRoad = *static_cast<Road*>(m_connections[0].connected);

	auto incomingLanes = connectedRoad.getAllLanesConnectingTo(this);
	auto outcomingLanes = connectedRoad.getAllLanesConnectingFrom(this);

	const auto axisPoints = m_shape.getAxisPoints();
	const auto shapeEndPoint = m_shape.getHead();

	// right paths
	for(const auto& path : outcomingLanes)
	{
		const auto pathStartPoint = path.points.front();
		auto direction = glm::normalize(pathStartPoint - shapeEndPoint);
		auto distance = glm::length(pathStartPoint - shapeEndPoint);

		Points pathPoints = getMovedPointsInDirectionAndDistance(axisPoints, direction, distance);

		Lane path;
		path.connectsFrom = nullptr;
		path.connectsTo = &connectedRoad;
		path.side = Lane::Side::RIGHT;
		path.points = pathPoints;

		m_lanes[Lane::Side::RIGHT].emplace_back(path);

	}
	// left paths
	for (const auto& path : incomingLanes)
	{
		const auto pathStartPoint = path.points.back();
		auto direction = glm::normalize(pathStartPoint - shapeEndPoint);
		auto distance = glm::length(pathStartPoint - shapeEndPoint);

		Points pathPoints = getMovedPointsInDirectionAndDistance(axisPoints, direction, distance);
		// reverse since its left
		std::reverse(pathPoints.begin(), pathPoints.end());

		Lane path;
		path.connectsFrom = &connectedRoad;
		path.connectsTo = nullptr;
		path.side = Lane::Side::LEFT;
		path.points = pathPoints;

		m_lanes[Lane::Side::LEFT].emplace_back(path);
	}
}

bool CarSpawner::canSwitchLanes() const
{
	return false;
}

void CarSpawner::construct(Road* road, Point connectPoint)
{
	connect(road, connectPoint);

	{
		auto connectDirection = -glm::normalize(connectPoint - road->getDirectionPointFromConnectionPoint(connectPoint));
		auto constructDirection = -glm::normalize(connectDirection);

	
		const float width = road->getWidth();
		Point centrePoint2 = connectPoint;
		Point centrePoint1 = connectPoint + constructDirection * width;

		// make them local
		{
			auto averagePos = (centrePoint1 + centrePoint2) / (2.0f);
			centrePoint1 -= averagePos;
			centrePoint2 -= averagePos;

			// and of course setPosition
			setPosition(averagePos);
		}
		Points axisPoints = { centrePoint1, centrePoint2 };

		m_shape = {};
		m_shape.construct(axisPoints, width);

		// makeShapePoints
		auto leftPoints = m_shape.getLeftSidePoints();
		auto rightPoints = m_shape.getRightSidePoints();
		m_shapePoints.resize(leftPoints.size() + rightPoints.size());

		auto copyIt = std::copy(leftPoints.begin(), leftPoints.end(), m_shapePoints.begin());
		std::reverse_copy(rightPoints.begin(), rightPoints.end(), copyIt);
	}


	VD::PositionVertices vertices = m_shapePoints;
	VD::ColorVertices colors(vertices.size(), glm::vec4(0.5f, 0.1f, 0.5f, 1.0));
	// since we go on ther circumreference
	VD::Indices indices = { 0,1,2, 2,3,0 };

	Mesh mesh;
	mesh.vertices.positions = vertices;
	mesh.vertices.colors = colors;
	mesh.indices = indices;

	Model model;
	model.meshes.push_back(mesh);

	Info::ModelInfo mInfo;
	mInfo.model = &model;

	setupModel(mInfo, true);
}

void CarSpawner::disable()
{
	m_disabled = true;
	m_spawnedCars.clear();
}

void CarSpawner::enable()
{
	m_disabled = false;
	m_spawnedCars.clear();
}

void CarSpawner::initializePossiblePaths(const std::vector<CarSpawner>& allSpawners)
{
	m_routesToSpawner.clear();
	for (const auto& spawner : allSpawners)
	{
		// dont make with self
		if (&spawner == this)
			continue;

		m_routesToSpawner[&spawner] = PathFinding::createRoadRoutes(*this, spawner);
	}
}

void CarSpawner::spawnCar()
{
	if (m_lanes.empty())
	{
		std::cout << "No lanes generated for spawner\n";
		return;
	}
	if (!m_routesToSpawner.empty())
	{
		const auto& [spawner, routes] = *m_routesToSpawner.begin();
		if (spawner->m_lanes.empty())
		{
			std::cout << "No lanes generated for other spawner\n";
			return;
		}

		if (!routes.empty())
		{
			auto route = routes.front();

			auto randomLane = [](const std::vector<Lane>& lanes) -> Lane
			{
				int maxNum = lanes.size() - 1;

				static std::mt19937 engine(std::chrono::high_resolution_clock::now().time_since_epoch().count());

				std::uniform_int_distribution randomDistributor(0, maxNum);

				return lanes[randomDistributor(engine)];
			};

			Lane startLane = randomLane(m_lanes[Lane::Side::RIGHT]);
			// left since we enter from opposite direction
			Lane endLane = randomLane(spawner->m_lanes.find(Lane::Side::LEFT)->second);

			auto travellSEgments = PathFinding::findLaneOnRoute(route, startLane, endLane);

			auto& car = m_spawnedCars.emplace_back(SimpleCar());
			car.drive(travellSEgments);
			car.setActive(true);
		}
	}
	std::cout << "\n";
}
