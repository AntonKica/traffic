#include "ObjectManager.h"
#include "SimulationArea.h"

ObjectManager::ObjectManager(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea), 
	m_roadCreator(this)// m_buildingCreator(this), 
{
	m_roadCreator.setActive(true);
}

ObjectManager::~ObjectManager()
{
}

void ObjectManager::initialize()
{
	//m_buildingCreator.prepareResources();
}

void ObjectManager::update()
{
	if (!m_disabledCreators)
	{
		if (m_currentCreator == CreatorType::ROAD)
		{
			m_roadCreator.setActive(true);
		}
		else
		{
			m_roadCreator.setActive(false);
		}

		m_roadCreator.update();
		//m_buildingCreator.update();
	}
}


void ObjectManager::setCreatorsModes(Creator::CreatorMode mode)
{
	m_currentCreatorMode = mode;

	m_roadCreator.setCreatorMode(mode);
	//m_buildingCreator.setCreatorMode(mode);
}

void ObjectManager::setCurrentCreator(CreatorType creatorType)
{
	m_currentCreator = creatorType;

	switch (m_currentCreator)
	{
	case ObjectManager::CreatorType::ROAD:
		m_roadCreator.setActive(false);
	}
}

void ObjectManager::drawUI()
{
	// format
	ImGui::Columns(2, 0, false);
	auto columnWidth = 120.0f;
	ImGui::SetColumnWidth(0, columnWidth);

	/*
		IMPLEMENT IMAGES LATER
	*/
	//auto image = UI::getInstance().loadImage("resources/menu/image.jpg");
	//ImGui::Image(image->image, ImVec2(250, 250));

	auto height = 200.0f;
	ImVec2 buttonSize(columnWidth - columnWidth * 0.1, height / 2.0f - height * 0.1);


	const ImVec4 greyColor(130 / 255.0, 130 / 255.0, 130 / 255.0, 255 / 255.0);
	const ImVec4 greenColor(125 / 255.0, 202 / 255.0, 24 / 255.0, 255 / 255.0);
	const ImVec4 greenishColor(125 / 255.0, 125 / 250.0, 125 / 250.0, 125 / 255.0);

	ImVec4 hoverColor;
	ImVec4 curColor;
	if (m_currentCreator == CreatorType::ROAD)
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
	if (ImGui::Button("Road", buttonSize))
		m_currentCreator = CreatorType::ROAD;

	ImGui::PopStyleColor(2);

	// self depended uis
	ImGui::NextColumn();
	ImGui::BeginChild(" child ");
	if (m_currentCreator == CreatorType::ROAD)
		m_roadCreator.drawUI();

	ImGui::EndChild();
}

void ObjectManager::disableCreators()
{
	m_roadCreator.setActive(false);
	//m_buildingCreator.setActive(false);

	m_disabledCreators = true;
}

void ObjectManager::enableCreators()
{
	m_roadCreator.setActive(false);
	//m_buildingCreator.setActive(true);

	m_disabledCreators = false;
}
