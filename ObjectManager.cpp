#include "ObjectManager.h"
#include "SimulationArea.h"


ObjectManagerUI::ObjectManagerUI(ObjectManager* objectManager, CreatorType activeCreator)
{
	m_pObjectManager = objectManager;

	m_currentCreator = activeCreator;
	m_shouldCreate = true;

	setActive(true);
}

void ObjectManagerUI::draw()
{
	auto io = ImGui::GetIO();

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("SelectMenu", 0, windowFlags);


	auto dispaySize = io.DisplaySize;
	auto windowWidth = ImGui::GetWindowWidth();

	auto height = 200.0f;

	ImGui::SetWindowSize(ImVec2(dispaySize.x, height));
	ImGui::SetWindowPos(ImVec2(0, dispaySize.y - height));
	// format
	ImGui::Columns(2, 0, false);
	auto columnWidth = 120.0f;
	ImGui::SetColumnWidth(0, columnWidth);
	
	/*
		IMPLEMENT IMAGES LATER
	*/
	//auto image = UI::getInstance().loadImage("resources/menu/image.jpg");
	//ImGui::Image(image->image, ImVec2(250, 250));

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
	ImGui::Checkbox("Create", &m_shouldCreate);
	if (m_shouldCreate)
		m_pObjectManager->setCreatorsModes(Creator::CreatorMode::CREATE);
	else
		m_pObjectManager->setCreatorsModes(Creator::CreatorMode::DESTROY);

	ImGui::PushStyleColor(ImGuiCol_Button, curColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
	if (ImGui::Button("Road", buttonSize))
		m_currentCreator = CreatorType::ROAD;

	if (m_currentCreator == CreatorType::BUILDING)
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
	if (ImGui::Button("Building", buttonSize))
		m_currentCreator = CreatorType::BUILDING;

	ImGui::PopStyleColor(4);

	// self depended uis
	ImGui::NextColumn();
	ImGui::BeginChild(" child ");
	if (m_currentCreator == CreatorType::ROAD)
		m_pObjectManager->m_roadCreator.drawUI();
	else
		m_pObjectManager->m_buildingCreator.drawUI();
	ImGui::End();

	ImGui::End();
}

ObjectManagerUI::CreatorType ObjectManagerUI::getCurrentCreator() const
{
	return m_currentCreator;
}


ObjectManager::ObjectManager(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea), 
	m_roadCreator(this), m_buildingCreator(this), 
	m_ui(this, ObjectManagerUI::CreatorType::ROAD)
{
	m_roadCreator.setActive(true);
}

ObjectManager::~ObjectManager()
{
}

void ObjectManager::initialize()
{
	m_buildingCreator.prepareResources();
}

void ObjectManager::update()
{
	if (!m_disabledCreators)
	{
		if (m_ui.getCurrentCreator() == ObjectManagerUI::CreatorType::ROAD)
		{
			m_roadCreator.setActive(true);
			m_buildingCreator.setActive(false);
		}
		else
		{
			m_roadCreator.setActive(false);
			m_buildingCreator.setActive(true);
		}

		m_roadCreator.update();
		m_buildingCreator.update();
	}
}


void ObjectManager::setCreatorsModes(Creator::CreatorMode mode)
{
	m_roadCreator.setCreatorMode(mode);
	m_buildingCreator.setCreatorMode(mode);
}

void ObjectManager::clickEvent()
{
}

void ObjectManager::disableCreators()
{
	m_roadCreator.setActive(false);
	m_buildingCreator.setActive(false);

	m_disabledCreators = true;

	m_ui.setActive(false);
}

void ObjectManager::enableCreators()
{
	m_roadCreator.setActive(false);
	m_buildingCreator.setActive(true);

	m_disabledCreators = false;
	m_ui.setActive(true);
}
