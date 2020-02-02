#include "CarSpawner.h"
#include "Road.h"
#include <numeric>

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

void CarSpawner::createPaths()
{
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

		auto leftPerpDir = glm::cross(constructDirection, glm::vec3(0.0, 1.0, 0.0));
		auto rightPerpDir = -leftPerpDir;
		float width = road->getWidth();
	
		Point leftPoint1 = connectPoint + leftPerpDir * width / 2.0f;
		Point rightPoint1 = connectPoint + rightPerpDir * width / 2.0f;
		Point leftPoint2 = leftPoint1 + constructDirection * width;
		Point rightPoint2 = rightPoint1 + constructDirection * width;
		
		// 
		m_shapePoints = { leftPoint1, rightPoint1, rightPoint2 , leftPoint2};

		// make them local and set position
		glm::vec3 sum = std::accumulate(std::begin(m_shapePoints), std::end(m_shapePoints), glm::vec3{});
		glm::vec3 newPos = sum / 4.0f;
		for (auto& sPoint : m_shapePoints)
			sPoint -= newPos;
		setPosition(newPos);
	}

	VD::PositionVertices vertices = m_shapePoints;
	VD::ColorVertices colors(vertices.size(), glm::vec4(0.5f, 0.0, 0.5f, 1.0));
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
}

void CarSpawner::enable()
{
	m_disabled = false;
}
