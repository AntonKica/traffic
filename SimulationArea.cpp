#include "SimulationArea.h"
#include "GlobalObjects.h"
#include <optional>

static std::optional<glm::vec3> planeIntersectPoint(const glm::vec3& rayDirection, const glm::vec3& rayPosition,
	const glm::vec3& planeNormal, const glm::vec3& planePosition)
{
	std::optional<glm::vec3> intersectionPoint;
	float directionAngle = dot(rayDirection, planeNormal);

	if (abs(directionAngle) > 0)
	{
		float dist = glm::length(planePosition - rayPosition);
		float hitDist = -(glm::dot(rayPosition, planeNormal)) / directionAngle;

		intersectionPoint = (planePosition + rayPosition) + (rayDirection * hitDist);
	}

	return intersectionPoint;
}

// on XZ plane
static std::pair<std::vector<glm::vec3>, std::vector<uint32_t>>generateCircleVerticesAndIndices(float size, int requestedPoints)
{
	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;

	// since first point needs to be done twice
	vertices.push_back(glm::vec3(0.0, 0.0, 0.0));
	for (int i = 0; i < requestedPoints; ++i)
	{
		float curAngle = glm::radians(360.0f / requestedPoints * i - 360.0f);

		glm::vec3 point = glm::vec3(glm::cos(curAngle), 0.0, glm::sin(curAngle)) * size;
		vertices.push_back(point);

		if (i >= 1)
		{
			std::array<uint32_t, 3> pts = { 0,i, i + 1};

			indices.insert(indices.begin(), pts.begin(), pts.end());
		}
	}
	// one last indice
	std::array<uint32_t, 3> pts = { 0,requestedPoints , 1 };
	indices.insert(indices.end(), pts.begin(), pts.end());

	return std::make_pair(vertices, indices);
}

SimulationAreaVisualizer::SimulationAreaVisualizer()
{
	position = glm::vec3(0.0);
}

void SimulationAreaVisualizer::createVisuals(size_t xCount, size_t zCount, double distanceBetweenPoints)
{
	const int maxXCount = 150;
	const int maxZCount = 155;
	const size_t xPointsCount = std::clamp<size_t>(xCount, 0, maxXCount);
	const size_t zPointsCount = std::clamp<size_t>(zCount, 0, maxZCount);
	const float sinkYCoord = -0.01;

	VD::PositionVertices vertices;
	VD::Indices indices;

	const auto [circleVertices, circleIndices] = generateCircleVerticesAndIndices(0.05, 4);

	vertices.resize(xPointsCount * zPointsCount * circleVertices.size());

	indices.resize(xPointsCount * zPointsCount * circleIndices.size());
	auto indIt = indices.begin();
	
	size_t vertexIndex = 0;
	size_t indexIndex = 0;
	for (int x = 0; x < xPointsCount; ++x)
	{
		for (int z = 0; z < zPointsCount; ++z)
		{
			glm::vec3 point;
			point.x = (-(float(xPointsCount) / 2.0f) + x) * distanceBetweenPoints;
			point.y = sinkYCoord;
			point.z = (-(float(zPointsCount) / 2.0f) + z) * distanceBetweenPoints;

			// vertices
			for (const auto& circlePoint : circleVertices)
				vertices[vertexIndex++] = point + circlePoint;

			// indices from reverse
			for (int i = circleIndices.size() - 1; i >= 0; --i)
			{
				size_t indexOffset = indexIndex * circleVertices.size();
				*indIt++ = circleIndices[i] + indexOffset;
			}
			++indexIndex;
		}
	}
	Mesh mesh;
	mesh.vertices.positions = vertices;
	mesh.indices = indices;

	Model model;
	model.meshes.push_back(mesh);

	Info::DrawInfo drawInfo;
	drawInfo.lineWidth = 1.0f;
	drawInfo.polygon = VK_POLYGON_MODE_FILL;
	drawInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


	Info::ModelInfo modelInfo;
	modelInfo.model = &model;

	Info::GraphicsComponentCreateInfo info;
	info.drawInfo = &drawInfo;
	info.modelInfo = &modelInfo;

	graphics = App::Scene.vulkanBase.createGrahicsComponent(info);
	graphics.setActive(true);
}

void SimulationAreaVisualizer::update()
{	
	if (!graphics.initialized())
		return;

	glm::vec3 camPos = App::Scene.m_camera.getPosition();
	camPos.y = 0;

	if (glm::length(camPos - position) > 10)
	{
		// no stutter
		position = glm::ivec3(App::Scene.m_simArea.getNearestPoint(camPos));

		graphics.setPosition(App::Scene.m_simArea.getNearestPoint(position));
	}
}


SimulationArea::SimulationArea()
	: m_objectManager(this)
{
}

SimulationArea::~SimulationArea()
{

}

void SimulationArea::initArea()
{
	initTraits();

	const auto [xCount, zCount] = getPointsCount();

	m_visuals.createVisuals(xCount, zCount, getDirectPointDistance());
}

void SimulationArea::loadData()
{
}

void SimulationArea::update()
{
	updateMousePosition();

	m_visuals.update();

	m_objectManager.update();
}

bool SimulationArea::placeObject()
{
	return false;
}

bool SimulationArea::isInArea(const glm::vec3& position) const
{
	const float halfWidth = (float(m_traits.AreaWidth) / 2.0f);
	const float halfHeight = (float(m_traits.AreaHeight) / 2.0f);

	return std::clamp(position.x, -halfWidth, halfWidth) == position.x && 
		std::clamp(position.z, -halfHeight, halfHeight) == position.z;
}

void SimulationArea::setEnableMouse(bool value)
{
	m_enableMouse = value;
}

void SimulationArea::clickEvent()
{
	if (m_enableMouse)
	{
		m_objectManager.clickEvent();
	}
}

std::pair<size_t, size_t> SimulationArea::getPointsCount() const
{
	size_t xCount = m_traits.AreaWidth * m_traits.PointsPerUnit;
	size_t zCount = m_traits.AreaHeight * m_traits.PointsPerUnit;

	return std::make_pair(xCount, zCount);
}

std::optional<glm::vec3> SimulationArea::getSelectedPointPos() const
{
	std::optional<glm::vec3> selectedPoint;
	if (m_mousePosition)
		selectedPoint = getNearestPoint(m_mousePosition.value());

	return selectedPoint;
}

std::optional<glm::vec3> SimulationArea::getMousePosition() const
{
	return m_mousePosition;
}

void SimulationArea::initTraits()
{
	m_traits.Position = SSA::origin;
	m_traits.UnitLength = SSA::unitLength;
	m_traits.PointsPerUnit = SSA::pointsPerUnit;

	m_traits.AreaWidth = SSA::defaultSize;
	m_traits.AreaHeight = SSA::defaultSize;
}

void SimulationArea::updateMousePosition()
{
	if (!m_enableMouse)
	{
		m_mousePosition.reset();
	}
	else
	{
		const glm::vec3 worldMouseRay = App::Scene.m_camera.getMouseRay();
		const glm::vec3 viewPosition = App::Scene.m_camera.getPosition();

		const glm::vec3 thisNormal = glm::vec3(0.0, 1.0, 0.0);

		const auto possibleIntersection =
			planeIntersectPoint(worldMouseRay, viewPosition, thisNormal, m_traits.Position);

		if (possibleIntersection)
		{
			m_mousePosition = possibleIntersection;
		}
		else
		{
			m_mousePosition.reset();
		}
	}
}

inline glm::vec3 SimulationArea::getNearestPoint(const glm::vec3& position) const
{
	const float pointDistance = getDirectPointDistance();

	auto spacedNumber = [](const float number, const float spaceSize) -> float
	{
		if (spaceSize <= 0 )
			throw std::runtime_error("negative or null space size");

		const int nearestDivider = std::round(number / spaceSize);
		const float nearestSpacedNumber = nearestDivider * spaceSize;

		return nearestSpacedNumber;
	};

	glm::vec3 nearestPosition;
	nearestPosition.x = spacedNumber(position.x, pointDistance);
	nearestPosition.y = position.y;
	nearestPosition.z = spacedNumber(position.z, pointDistance);

	return nearestPosition;
}

inline float SimulationArea::getDirectPointDistance() const
{
	return m_traits.UnitLength / m_traits.PointsPerUnit;
}
