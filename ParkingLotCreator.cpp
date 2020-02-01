#include "ParkingLotCreator.h"
#include "BuildingCreator.h"
#include "SimulationArea.h"
#include "GlobalObjects.h"

ParkingLotCreator::ParkingLotCreator(BuildingCreator* pBuildingCreator, SimulationArea* pSimulationArea)
	:m_pBuildingCreator(pBuildingCreator), m_pSimulationArea(pSimulationArea)
{
}

void ParkingLotCreator::update()
{
	if (App::input.mouse.pressedButton(GLFW_MOUSE_BUTTON_LEFT))
	{
		if (m_currentEdge)
		{
			const auto& cps = m_currentEdge.value();
			m_settedEdges.emplace_back(m_currentEdge.value());

			tryCreatingParkingLot();
		}
	}

	updateCurrentEdges();
	updatePrototype();
	updateHelpLines();
}

void ParkingLotCreator::setActiveAction()
{
	if (!isActive())
	{
		m_settedEdges.clear();
		m_currentEdge.reset();
	}

	// disable prototype
	m_prototype.setActive(isActive());
}

void ParkingLotCreator::updateCurrentEdges()
{
	auto mousePos = m_pSimulationArea->getMousePosition();
	auto selectedObject = m_pSimulationArea->getSelectedObject();

	if (selectedObject && mousePos)
	{
		if (auto road = dynamic_cast<Road*>(selectedObject.value()))
		{
			auto circumRefPoint = road->getCircumreferencePoint(mousePos.value());

			auto edgeDir = glm::normalize(circumRefPoint - mousePos.value());

			m_currentEdge = { circumRefPoint, edgeDir };
		}
		else
		{
			m_currentEdge.reset();
		}
	} 
	else
	{
		m_currentEdge.reset();
	}
}

void ParkingLotCreator::updatePrototype()
{
	if (m_currentEdge && !m_settedEdges.empty())
	{
		m_prototype.construct({ m_settedEdges[0], m_currentEdge.value() });
	}
	else
	{
		m_prototype.setActive(false);
	}
}

void ParkingLotCreator::updateHelpLines()
{
	if (!m_currentEdge && m_settedEdges.empty())
	{
		disableGraphics();
	}
	else
	{
		enableGraphics();

		Points edgePoints;
		if (!m_settedEdges.empty())
		{
			edgePoints.emplace_back(m_settedEdges[0].position);
			Point pt = m_settedEdges[0].position + m_settedEdges[0].direction * 3.0f;
			edgePoints.emplace_back(pt);
		}
		if (m_currentEdge)
		{
			edgePoints.emplace_back(m_currentEdge.value().position);
			Point pt = m_currentEdge.value().position + m_currentEdge.value().direction * 3.0f;
			edgePoints.emplace_back(pt);
		}

		VD::Indices indices;
		if (edgePoints.size() == 2)
			indices = { 0, 1 };
		else
			indices = { 0, 1, 2, 3 };

		Mesh mesh;
		mesh.vertices.positions = VD::PositionVertices(std::begin(edgePoints), std::end(edgePoints));
		mesh.vertices.colors = VD::ColorVertices(edgePoints.size(), glm::vec4(1.0, 0.0, 0.0, 1.0));
		mesh.indices = indices;

		Model model;
		model.meshes.emplace_back(mesh);

		Info::ModelInfo mInfo;
		mInfo.model = &model;

		setupModelWithLines(mInfo, true);
	}
}

void ParkingLotCreator::tryCreatingParkingLot()
{
	if (m_settedEdges.size() == 2)
	{
		ParkingLot* plot = new ParkingLot(m_prototype);

		m_prototype.construct({});

		m_settedEdges.clear();
	}
}
