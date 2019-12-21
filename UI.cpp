#include "UI.h"
#include "VulkanBase.h"
#include "GlobalObjects.h"

UI& UI::getInstance()
{
	static UI s_ui;

	return s_ui;
}

void UI::initUI(VulkanBase* pVulkanBase)
{
	vulkanBase = pVulkanBase;
	device = vulkanBase->getDevice();
	window = vulkanBase->getWindow();
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

void UI::registerUIElement(UIElement* element)
{
	m_UIElements.push_back(element);
}

void UI::unregisterUIElement(UIElement* element)
{
	m_UIElements.erase(std::find(std::begin(m_UIElements), std::end(m_UIElements), element), m_UIElements.end());
}

void UI::drawUI(VkCommandBuffer cmdBuffer)
{
	if (m_UIElements.empty())
		return;

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

	GUI::newFrame(window, App::time.deltaTime());
	ImGui::NewFrame();

	for (auto& element : m_UIElements)
	{
		if (element->active())
			element->draw();
	}

	ImGui::Render();

	GUI::drawRenderData(*device, imgui, ImGui::GetDrawData(), cmdBuffer);
}

bool UI::mouseOverlap() const
{
	return ImGui::IsAnyWindowHovered();
}


UI::UI()
{
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

UIElement::UIElement()
{
	UI& instance = UI::getInstance();
	instance.registerUIElement(this);
}

UIElement::~UIElement()
{
	UI& instance = UI::getInstance();
	instance.unregisterUIElement(this);
}

UIElement::UIElement(const UIElement& otherElement)
{
	UI& instance = UI::getInstance();
	instance.registerUIElement(this);

	m_active = otherElement.m_active;
}

UIElement::UIElement(UIElement&& otherElement)
{
	UI& instance = UI::getInstance();
	instance.registerUIElement(this);
	instance.unregisterUIElement(this);

	m_active = otherElement.m_active;
}

UIElement& UIElement::operator=(const UIElement& otherElement)
{
	UI& instance = UI::getInstance();
	instance.registerUIElement(this);

	return *this;
}

UIElement& UIElement::operator=(UIElement&& otherElement)
{
	UI& instance = UI::getInstance();
	instance.registerUIElement(this);
	instance.unregisterUIElement(&otherElement);

	return *this;
}

bool UIElement::active() const
{
	return m_active;
}

void UIElement::setActive(bool active)
{
	m_active = active;
}

