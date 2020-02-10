#include "SimulationArea.h"
#include "GlobalObjects.h"
#include "SimpleCar.h"
#include <optional>
#include "RoadPathFinder.h"

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

SimulationAreaVisualizer::SimulationAreaVisualizer(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea), position(glm::vec3())
{
}

void SimulationAreaVisualizer::createVisuals(size_t xCount, size_t zCount, double distanceBetweenPoints)
{
	const int maxXCount = 500;
	const int maxZCount = 500;
	const size_t xPointsCount = std::clamp<size_t>(xCount, 0, maxXCount);
	const size_t zPointsCount = std::clamp<size_t>(zCount, 0, maxZCount);
	constexpr const float sinkYCoord = -0.03;

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

	graphics.updateGraphicsComponent(info);
	graphics.setActive(true);
}

void SimulationAreaVisualizer::update()
{	
	if (!graphics.isActive())
		return;

	glm::vec3 camPos = App::camera.getPosition();
	camPos.y = 0;

	if (glm::length(camPos - position) > 10)
	{
		// no stutter
		position = glm::ivec3(m_pSimulationArea->getNearestPoint(camPos));

		graphics.setPosition(m_pSimulationArea->getNearestPoint(position));
	}
}


SimulationArea::SimulationArea()
	: m_visuals(this), m_objectManager(this), m_topMenu(this), m_bottomMenu(this), m_roadInspector(this)
{
	m_enableMouse = true;

	setSimualtionMode(SimulationMode::EDIT);
}

SimulationArea::~SimulationArea()
{
	m_currentMode = Mode::CREATE;
}

void SimulationArea::initArea()
{
	initTraits();

	const auto [xCount, zCount] = getPointsCount();

	m_visuals.createVisuals(xCount, zCount, getDirectPointDistance());
	m_objectManager.initialize();
}

void SimulationArea::loadData()
{
}

void SimulationArea::update()
{
	m_visuals.update();
	
	handleCurrentMode();
}

void SimulationArea::handleCurrentMode()
{
	switch (m_currentMode)
	{
	case SimulationArea::Mode::CREATE:
	case SimulationArea::Mode::INSPECT:
	case SimulationArea::Mode::DESTROY:
		m_objectManager.update();
		updateMousePosition();
		updateSelecteObject();
	}
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

void SimulationArea::setMode(Mode mode)
{
	m_currentMode = mode;

	switch (m_currentMode)
	{
	case SimulationArea::Mode::CREATE:
		m_objectManager.enableCreators();
		m_objectManager.setCreatorsModes(Creator::CreatorMode::CREATE);

		m_roadInspector.setActive(false);
		break;
	case SimulationArea::Mode::INSPECT:
		m_objectManager.disableCreators();

		m_roadInspector.setActive(true);
		break;
	case SimulationArea::Mode::DESTROY:
		m_objectManager.enableCreators();
		m_objectManager.setCreatorsModes(Creator::CreatorMode::DESTROY);

		m_roadInspector.setActive(false);
		break;
	}
}


void SimulationArea::setSimualtionMode(SimulationMode simulationMode)
{
	m_currentSimulationMode = simulationMode;

	switch (m_currentSimulationMode)
	{
	case SimulationArea::SimulationMode::RUN:
		m_objectManager.disableCreators();
		m_bottomMenu.setActive(false);


		// force mode
		setMode(Mode::INSPECT);

		m_roadInspector.setInspectorMode(Inspector::InspectorMode::INSPECT);
		runSimulation();
		break;

	case SimulationArea::SimulationMode::EDIT:
		stopSimulation();
		m_bottomMenu.setActive(true);

		// force mode
		setMode(Mode::CREATE);

		m_roadInspector.setInspectorMode(Inspector::InspectorMode::EDIT);
		m_objectManager.enableCreators();
		break;
	}
}

SimulationArea::Mode SimulationArea::getMode() const
{
	return m_currentMode;
}

SimulationArea::SimulationMode SimulationArea::getSimualtionMode() const
{
	return m_currentSimulationMode;
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

void SimulationArea::updateSelecteObject()
{
	// lets fake it for now
	m_selectedObject.reset();

	const glm::vec4 green = glm::vec4(0.1, 0.98, 0.25, 1.0);
	if (m_mousePosition)
	{
		auto adjustTransparencyIfPointSitsOn = [&](BasicRoad& basicRoad)
		{
			if (basicRoad.sitsPointOn(m_mousePosition.value()))
			{
				m_selectedObject = &basicRoad;
				basicRoad.getGraphicsComponent().setTransparency(0.5);
			}
			else
			{
				//road.getGraphicsComponent().setTint(glm::vec4());
				basicRoad.getGraphicsComponent().setTransparency(0);
			}
		};

		for (auto& road : m_objectManager.m_roads.data)
			adjustTransparencyIfPointSitsOn(road);
	

		for (auto& intersection : m_objectManager.m_intersections.data)
			adjustTransparencyIfPointSitsOn(intersection);
		

		for (auto& roadSpawner : m_objectManager.m_carSpawners.data)
			adjustTransparencyIfPointSitsOn(roadSpawner);
	}
}

std::optional<SimulationObject*> SimulationArea::getSelectedObject() const
{
	return m_selectedObject;
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
	if (!m_enableMouse || UI::getInstance().mouseOverlap())
	{
		m_mousePosition.reset();
	}
	else
	{
		const glm::vec3 worldMouseRay = App::camera.getMouseRay();
		const glm::vec3 viewPosition = App::camera.getPosition();

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

Point closestTraiPoint(const Trail& trail, Point pt)
{
	// find edges
	const auto [p1, p2] = findTwoClosestPoints(trail, pt);
	return getClosestPointToLineSegment(p1, p2, pt);
}

/*
const Road* SimulationArea::findClosestRoadFromBuilding(const BasicBuilding& building) const
{
	const auto buildingPos = building.getPosition();

	const Road* closestRoad = nullptr;
	float closestDistance= {};
	for (const auto& road : m_objectManager.m_roads.data)
	{
		auto axis = road.getAxis();
		Points pts(std::begin(axis), std::end(axis));

		auto pt = closestTraiPoint(pts, buildingPos);
		if (!closestRoad || glm::length(buildingPos - pt) < closestDistance)
		{
			closestRoad = &road;
			closestDistance = glm::length(buildingPos - pt);
		}
	}

	return closestRoad;
}


void SimulationArea::connectBuildingsAndRoads()
{
	for (auto& building : m_objectManager.m_houses.data)
		connectBuildingToClosestRoad(building);
}

void SimulationArea::connectBuildingToClosestRoad(BasicBuilding& building)
{
	const auto buildingPos = building.getPosition();

	Road* closestRoad = nullptr;
	Point closestRoadPoint = {};
	for (auto& road : m_objectManager.m_roads.data)
	{
		const auto roadAxis = road.getAxisPoints();
		const auto roadPoint = closestTraiPoint(roadAxis, buildingPos);

		if (!closestRoad)
		{
			closestRoad = &road;
			closestRoadPoint = roadPoint;
		}
		else
		{
			auto closestDistance = glm::length(buildingPos - closestRoadPoint);
			auto currentDistance = glm::length(buildingPos - roadPoint);

			if (currentDistance < closestDistance)
			{
				closestRoad = &road;
				closestRoadPoint = roadPoint;
			}
		}
	}

	// set
	if (closestRoad)
	{
		closestRoad->addNearbyByuilding(&building, closestRoadPoint);
		building.setNearbyRoad(closestRoad, closestRoadPoint);
	}
}
*/

class PathVisualizer : public SimulationObject
{
public:
	void addPoints(Points pts)
	{
		m_pts.push_back(pts);
	}

	void setupDraws()
	{
		Model model;

		uint32_t count = 0;
		for (const auto& points : m_pts)
		{
			glm::vec4 color;
			if (count == 0)
				color = glm::vec4(1.0, 0.0, 0.0,1.0);
			else if (count == 1)
				color = glm::vec4(0.0, 1.0, 0.0, 1.0);
			else if (count == 2)
				color = glm::vec4(0.0, 0.0, 1.0, 1.0);
			else if (count == 3)
				color = glm::vec4(1.0, 1.0, 0.0, 1.0);
			else if (count == 4)
				color = glm::vec4(0.0, 1.0, 1.0, 1.0);
			else if (count == 5)
				color = glm::vec4(1.0, 1.0, 1.0, 1.0);
			++count;
			Mesh newMesh;
			newMesh.vertices.positions = points;
			newMesh.vertices.colors = VD::ColorVertices(points.size(), color);
			for (uint32_t indexOne = 0, indexTwo = 1; indexTwo < points.size(); ++indexOne, ++indexTwo)
			{
				newMesh.indices.emplace_back(indexOne);
				newMesh.indices.emplace_back(indexTwo);
			}

			model.meshes.push_back(newMesh);
		}

		Info::ModelInfo minfo;
		minfo.model = &model;

		setupModelWithLines(minfo, true);
		setActive(true);
	}

	std::vector<Points> m_pts;
};

void SimulationArea::runSimulation()
{
	// reset and setup
	{
		// first setup roads then inrersections since intersection rely on 
		// road paths being setup and spawners as well
		for (auto& road : m_objectManager.m_roads.data)
		{
			road.createLanes();
			//road.resetNearbyBuildings();
		}

		//static PathVisualizer visualizer;
		for (auto& intersection : m_objectManager.m_intersections.data)
		{
			intersection.createLanes();
		}
		//visualizer.setupDraws();
		for (auto& spawner : m_objectManager.m_carSpawners.data)
		{
			spawner.createLanes();
		}
	}

	// init spawners
	{
		for(auto & spawner : m_objectManager.m_carSpawners.data)
		{
			spawner.initializePossiblePaths(m_objectManager.m_carSpawners.data);
		}
	}
	{
		for (auto& spawner : m_objectManager.m_carSpawners.data)
		{
			spawner.enable();
			spawner.spawnCar();
		}
	}
	// EXPERIMENT
	/*

		// then connect
	{
		connectBuildingsAndRoads();
	}

	{
		auto& roads = m_objectManager.m_roads.data;
		auto& r1 = roads.front();
		auto& r2 = roads.back();
		auto route = createRoadRoutes(&r1, &r2);

		if (!route.empty())
		{
			std::cout << "Found" << route.size() << "Routes\n";
			const auto& firstRoute = route.front();

			if (!r1.getNearbyBuildings().empty() && !r2.getNearbyBuildings().empty())
			{
				auto path = findPathOnRoute(firstRoute, r1.getNearbyBuildings()[0].entryPoint, r2.getNearbyBuildings()[0].entryPoint);
			}
		}
	}*/
	// cars last
	/*{
		exampleCars.clear();
		for (auto& house : m_objectManager.m_houses.data)
		{
			if (auto road = house.getNearbyRoad())
			{
				SimpleCar car;
				car.setActive(true);
				car.setPosition(house.getPosition());
				car.drive(*road);

				exampleCars.push_back(car);
			}
		}
	}*/
}

void SimulationArea::stopSimulation()
{
	for (auto& spawner : m_objectManager.m_carSpawners.data)
	{
		spawner.disable();
	}
}

TopMenu::TopMenu(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea)
{
	setActive(true);
}

void TopMenu::draw()
{
	auto io = ImGui::GetIO();

	ImGui::SetNextWindowBgAlpha(1.0);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("TopMenu", 0, windowFlags);


	auto dispaySize = io.DisplaySize;
	auto windowWidth = ImGui::GetWindowWidth();

	auto height = 50.0f;

	ImGui::SetWindowSize(ImVec2(dispaySize.x, height));
	ImGui::SetWindowPos(ImVec2(0, 0));

	ImVec2 buttonSize(height * 0.9, height * 0.9);


	const ImVec4 greyColor(130 / 255.0, 130 / 255.0, 130 / 255.0, 255 / 255.0);
	const ImVec4 greenColor(125 / 255.0, 202 / 255.0, 24 / 255.0, 255 / 255.0);

	ImVec4 curColor;
	if (m_pressedPlay)
		curColor = greenColor;
	else
		curColor = greyColor;

	ImGui::PushStyleColor(ImGuiCol_Button, curColor);
	ImGui::SetCursorPos(ImVec2(windowWidth / 2 - 40, height - buttonSize.y));
	if (ImGui::Button("Play", buttonSize))
	{
		m_pressedPlay = true;
		m_pSimulationArea->setSimualtionMode(SimulationArea::SimulationMode::RUN);
	}

	if (!m_pressedPlay)
		curColor = greenColor;
	else
		curColor = greyColor;

	ImGui::SetCursorPos(ImVec2(windowWidth / 2 + 40, height - buttonSize.y));
	ImGui::PushStyleColor(ImGuiCol_Button, curColor);
	if (ImGui::Button("Stop", buttonSize))
	{
		m_pressedPlay = false;
		m_pSimulationArea->setSimualtionMode(SimulationArea::SimulationMode::EDIT);
	}

	ImGui::PopStyleColor(2);

	ImGui::End();
}

bool TopMenu::pressedPlay() const
{
	return m_pressedPlay;
}

BottomMenu::BottomMenu(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea)
{
}

void BottomMenu::draw()
{
	auto& io = ImGui::GetIO();

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Bottom menu", 0, windowFlags);


	auto dispaySize = io.DisplaySize;
	auto windowWidth = ImGui::GetWindowWidth();

	auto height = 200.0f;
	ImGui::SetWindowSize(ImVec2(dispaySize.x, height));
	ImGui::SetWindowPos(ImVec2(0, dispaySize.y - height));

	ImGui::Columns(2, 0, false);
	float columnWidth = 80.0f;
	ImGui::SetColumnWidth(0, columnWidth);

	uint32_t pushCount = 0;
	auto setButtonStyleIfCompatibleModels = [&pushCount](SimulationArea::Mode modeOne, SimulationArea::Mode modeTwo)
	{
		const ImVec4 greyColor(130 / 255.0, 130 / 255.0, 130 / 255.0, 255 / 255.0);
		const ImVec4 greenColor(125 / 255.0, 202 / 255.0, 24 / 255.0, 255 / 255.0);
		const ImVec4 greenishColor(125 / 255.0, 125 / 250.0, 125 / 250.0, 125 / 255.0);

		ImVec4 hoverColor;
		ImVec4 curColor;

		if (modeOne == modeTwo)
		{
			hoverColor = curColor = greenColor;
		}
		else
		{
			hoverColor = greenishColor;
			curColor = greyColor;
		}

		ImGui::PushStyleColor(ImGuiCol_Button, curColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);

		pushCount += 2;
	};
	auto popButtonStyles = [&pushCount]()
	{
		ImGui::PopStyleColor(pushCount);
	};

	// buttons for such things
	{
		const ImVec2 buttonSize(columnWidth - 0.2f * columnWidth, columnWidth - 0.2f * columnWidth);

		setButtonStyleIfCompatibleModels(SimulationArea::Mode::CREATE, m_pSimulationArea->getMode());
		if (ImGui::Button("Create", buttonSize))
			m_pSimulationArea->setMode(SimulationArea::Mode::CREATE);

		setButtonStyleIfCompatibleModels(SimulationArea::Mode::DESTROY, m_pSimulationArea->getMode());
		if (ImGui::Button("Destroy", buttonSize))
			m_pSimulationArea->setMode(SimulationArea::Mode::DESTROY);

		setButtonStyleIfCompatibleModels(SimulationArea::Mode::INSPECT, m_pSimulationArea->getMode());
		if (ImGui::Button("Inspect", buttonSize))
			m_pSimulationArea->setMode(SimulationArea::Mode::INSPECT);

		popButtonStyles();
	}

	// adjust size so it wont show that much if we dont create
	if (m_pSimulationArea->getMode() != SimulationArea::Mode::CREATE)
		ImGui::SetWindowSize(ImVec2(columnWidth, height));

	ImGui::NextColumn();
	if (m_pSimulationArea->getSimualtionMode() == SimulationArea::SimulationMode::EDIT)
	{
		ImGui::BeginChild("ObjectManager");
		m_pSimulationArea->m_objectManager.drawUI();
		ImGui::EndChild();
	}

	ImGui::End();
}
