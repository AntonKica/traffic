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
	const int maxZCount = 150;
	size_t xPointsCount = std::clamp<size_t>(xCount, 0, maxXCount);
	size_t zPointsCount = std::clamp<size_t>(zCount, 0, maxZCount);

	GO::TypedVertices points;
	GO::Indices indices;
	auto& [vertType, variantVertices] = points;

	const auto [circleVertices, circleIndices] = generateCircleVerticesAndIndices(0.1, 5);

	variantVertices.resize(xPointsCount * zPointsCount * circleVertices.size());
	vertType = GO::VertexType::DEFAULT;

	indices.resize(variantVertices.size() * circleIndices.size());
	auto indIt = indices.begin();
	
	size_t currentIndex = 0;
	for (int x = 0; x < xPointsCount; ++x)
	{
		for (int z = 0; z < zPointsCount; ++z)
		{
			glm::vec3 point;
			point.x = -(float(xPointsCount) / 2.0f) + x;
			point.y = 0;
			point.z = -(float(zPointsCount) / 2.0f) + z;

			for (const auto& circlePoint : circleVertices)
			{
				glm::vec3 p = point + circlePoint;

				GO::VariantVertex varVert;
				varVert.vertex.position = p;
				variantVertices[currentIndex++] = varVert;
			}
			{
				indIt = std::transform(circleIndices.begin(), circleIndices.end(), indIt, [currentIndex](const uint32_t& i) {return i + currentIndex; });
			}

		}
	}

	Info::DrawInfo drawInfo;
	drawInfo.lineWidth = 1.0f;
	drawInfo.polygon = VK_POLYGON_MODE_FILL;
	drawInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	Info::ModelInfo modelInfo;
	modelInfo.vertices = &points;
	modelInfo.indices = &indices;

	Info::GraphicsComponentCreateInfo info;
	info.drawInfo = &drawInfo;
	info.modelInfo = &modelInfo;

	graphics = App::Scene.vulkanBase.createGrahicsComponent(info);
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
{
}

void SimulationArea::initArea()
{
	initTraits();

	const auto [xCount, zCount] = getPointsCount();

	//m_visuals.createVisuals(xCount, zCount, getDirectPointDistance());
}

void SimulationArea::loadData()
{
}

void SimulationArea::update()
{
	updateMousePosition();
	m_visuals.update();
	m_creator.update();
}

bool SimulationArea::placeObject()
{
	return false;
}

bool SimulationArea::placeSelectedObject()
{
	auto newObject = m_creator.getCurrentObject();
	if (newObject)
	{
		if (isInArea(newObject->getPosition()))
		{
			SimulationAreaObject* newObj = m_creator.getRawPointerFromType(m_creator.getCurrentType());
			*newObj = *newObject;
			newObj->updateGraphics();
			newObj->m_graphicsComponent.setActive(true);

			auto it = m_data.objects.insert(m_data.objects.begin(), newObj);
			
			std::cout << "Created obj\n";
			return true;
		}
	}
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
	if(m_enableMouse)
		placeSelectedObject();
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
	//const float xDecimal = position.x - std::floor(position.x);
	//const float zDecimal = position.z - std::floor(position.z);
	const float pointDistance = getDirectPointDistance();

	auto spacedNumber = [](const float number, const float spaceSize) -> float
	{
		if (spaceSize <= 0 )
			assert("negative or null space size");
		//const float negCompensator = number >= 0 ? 1 : -1;

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