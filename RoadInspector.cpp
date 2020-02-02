#include "RoadInspector.h"
#include "Road.h"
#include "RoadIntersection.h"
#include "CarSpawner.h"
#include "SimulationArea.h"
#include "GlobalObjects.h"

void RoadInspectorUI::draw()
{
	// nothing to insepct, nothing to draw
	if (!m_currentlyInspected)
		return;

	{
		float defaultWidth = 300.0f;
		float defaultHeight = 150.0f;
		ImGui::SetNextWindowSize(ImVec2(defaultWidth, defaultHeight));
	}

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("InspectorWindow", 0, windowFlags);

	if (m_resetPositionOnNextDraw)
	{
		auto& io = ImGui::GetIO();
		auto displaySize = io.DisplaySize;

		float xPos = displaySize.x - ImGui::GetWindowSize().x;
		float yPos = ImGui::GetWindowSize().y / 2.0f;
		ImGui::SetWindowPos(ImVec2(xPos, yPos));

		m_resetPositionOnNextDraw = false;
	}

	if (m_mode == Inspector::InspectorMode::EDIT)
		editCurrent();
	else if (m_mode == Inspector::InspectorMode::INSPECT)
		inspectCurrent();

	ImGui::End();
}

void RoadInspectorUI::setActiveAction()
{
	m_resetPositionOnNextDraw = true;
}

void RoadInspectorUI::setInspectElement(BasicRoad* inspected)
{
	if (inspected)
	{
		m_currentlyInspected = inspected;
		setActive(true);
	}
	else
	{
		m_currentlyInspected.reset();
		setActive(false);
	}
}

void RoadInspectorUI::resetInspectElement()
{
	m_currentlyInspected.reset();
}

void RoadInspectorUI::setMode(Inspector::InspectorMode mode)
{
	m_mode = mode;
}

void RoadInspectorUI::inspectCurrent()
{
	auto& inspectedRoad = m_currentlyInspected.value();

	if (inspectedRoad->getRoadType() == Road::RoadType::ROAD)
		inspectRoad(static_cast<Road*>(inspectedRoad));
	else if (inspectedRoad->getRoadType() == Road::RoadType::INTERSECTION)
		inspectIntersection(static_cast<RoadIntersection*>(inspectedRoad));
	else if (inspectedRoad->getRoadType() == Road::RoadType::CAR_SPAWNER)
		inspectSpawner(static_cast<CarSpawner*>(inspectedRoad));
}

void RoadInspectorUI::inspectRoad(Road* road)
{
	auto& roadParameters = road->m_parameters;

	ImGui::Text("Road");
	ImGui::NewLine();

	std::string laneCountText = "Lane count " + std::to_string(roadParameters.laneCount);
	ImGui::Text(laneCountText.c_str());
	std::string laneWidthText = "Lane width " + std::to_string(road->getWidth());
	ImGui::Text(laneWidthText.c_str());
	std::string pathLengthText = "Path length " + std::to_string(road->getLength());
	ImGui::Text(pathLengthText.c_str());
}

void RoadInspectorUI::inspectIntersection(RoadIntersection* roadIntersection)
{
	ImGui::Text("Intersection");
	ImGui::NewLine();

	std::string roadWayText = std::to_string(roadIntersection->directionCount()) + " - way intersection";
	ImGui::Text(roadWayText.c_str());
}

void RoadInspectorUI::inspectSpawner(CarSpawner* carSpawner)
{
	ImGui::Text("Spawner");
	ImGui::NewLine();
}

void RoadInspectorUI::editCurrent()
{
	if (!m_currentlyInspected)
	{
		setActive(false);
		return;
	}

	auto& inspectedRoad = m_currentlyInspected.value();

	if (inspectedRoad->getRoadType() == Road::RoadType::ROAD)
		editRoad(static_cast<Road*>(inspectedRoad));
	else if (inspectedRoad->getRoadType() == Road::RoadType::INTERSECTION)
		editIntersection(static_cast<RoadIntersection*>(inspectedRoad));
	else if (inspectedRoad->getRoadType() == Road::RoadType::CAR_SPAWNER)
		editSpawner(static_cast<CarSpawner*>(inspectedRoad));
}

// for now
void RoadInspectorUI::editRoad(Road* road)
{
	inspectRoad(road);
}

void RoadInspectorUI::editIntersection(RoadIntersection* roadIntersection)
{
	inspectIntersection(roadIntersection);
}

void RoadInspectorUI::editSpawner(CarSpawner* carSpawner)
{
	inspectSpawner(carSpawner);
}

RoadInspector::RoadInspector(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea)
{
}

void RoadInspector::update()
{
	updateCurrentlyInspected();
	processInput();
}

void RoadInspector::setActiveAction()
{
	m_currentlyInspected = nullptr;
	m_inspectorUI.resetInspectElement();

	m_inspectorUI.setActive(isActive());
}

void RoadInspector::setInspectorMode(Inspector::InspectorMode inspectorMode)
{
	m_currentInspectorMode = inspectorMode;
}

void RoadInspector::processInput()
{
	if (App::input.mouse.pressedButton(GLFW_MOUSE_BUTTON_LEFT) && !UI::getInstance().mouseOverlap())
		m_inspectorUI.setInspectElement(m_currentlyInspected);
	if (App::input.keyboard.pressedKey(GLFW_KEY_ESCAPE))
		m_inspectorUI.resetInspectElement();
}

void RoadInspector::updateCurrentlyInspected()
{
	m_currentlyInspected = nullptr;

	auto selectedObject = m_pSimulationArea->getSelectedObject();
	if (selectedObject)
	{
		if(auto basicRoad = dynamic_cast<BasicRoad*>(selectedObject.value()))
			m_currentlyInspected = basicRoad;
	}
}

