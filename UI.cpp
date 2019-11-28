#include "UI.h"
#include "VulkanBase.h"
#include "GlobalObjects.h"

void UI::initUI(VulkanBase* pVulkanBase)
{
	vulkanBase = pVulkanBase;
	device = vulkanBase->getDevice();
	window = vulkanBase->getWindow();

	selectedNum = 0;
	hidden = false;
	imgui = {};


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// create cpmtext
	ImGuiIO& io = ImGui::GetIO();

	// setup visuals
	ImGui::StyleColorsDark();


	createResources();
}

void UI::destroyUI()
{
	imgui.cleanup(*device, nullptr);
	ImGui::DestroyContext();
}

void UI::drawUI(VkCommandBuffer cmdBuffer)
{
	if (hidden)
	{
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() &&
		"Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	int width, height;
	int displayWidth, displayHeight;
	glfwGetWindowSize(window, &width, &height);
	glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
	io.DisplaySize = ImVec2(width, height);
	if (width > 0 && height > 0)
		io.DisplaySize =
		ImVec2(float(displayWidth) / width, float(displayHeight) / height);
	// : ) 
	double deltaTime = 0.0069;
	GUI::newFrame(window, deltaTime);
	ImGui::NewFrame();

	createBoxes();

	ImGui::Render();

	GUI::drawRenderData(*device, imgui, ImGui::GetDrawData(), cmdBuffer);
}

int UI::getSelection() const
{
	return selectedNum;
}

bool UI::mouseOverlap() const
{
	return ImGui::IsAnyWindowHovered();
}


void UI::createResources()
{
	VkRenderPass renderPass = vulkanBase->getRenderPass();
	// init basics
	GUI::createDeviceObjects(*device, renderPass, imgui);

	// inputs and callbacks
	GUI::initGlfwImplementation(imgui);
	// init texutres for font
	GUI::createFontsTexture(*device, imgui);
}

// or what is it called

void UI::createBoxes()
{
	// First box
	{
		static float f = 0.0f;

		ImGui::Begin("Menu window!");

		auto objectNames = App::Scene.m_simArea.m_roadManager.roadCreator.getRoadNames();

		int num = 0;
		for (const auto& name : objectNames)
		{
			if (ImGui::Button(name.c_str()))
			{
				selectedName = name;
				selectedNum = num + 1;

				App::Scene.m_simArea.m_roadManager.roadCreator.setPrototype(num);
			}
			++num;
		}
		static bool curvedRoads = false;
		ImGui::Checkbox("Curved roads", &curvedRoads);

		//ImGui::SameLine();
		ImGui::Text("Currently selected = %i %s", selectedNum + 1, selectedName.c_str());
		App::Scene.m_simArea.m_roadManager.roadCreator.setMode(curvedRoads ? 1 : 0);
		/*auto tile = App::Scene.m_grid.getSelectedTile();
		if (tile)
		{
			auto pos = tile->getPosition();
			ImGui::Text((std::string("Currently selected pos ") + glm::to_string(pos)).c_str());
		}*/
		auto pos = App::Scene.m_camera.getPosition();
		ImGui::Text((std::string("Currently selected pos ") + glm::to_string(pos)).c_str());
		ImGui::End();
	}
}
