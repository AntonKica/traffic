#include "UI.h"
#include "VulkanBase.h"
#include "GlobalObjects.h"
#include <stb/stb_image.h>

UI& UI::getInstance()
{
	static UI s_ui;

	return s_ui;
}

void UI::initUI(VulkanBase* pVulkanBase)
{
	vulkanBase = pVulkanBase;
	device = vulkanBase->getDevice();
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
	for (auto& [name, image] : m_loadedImages)
		image.cleanUp(*device, nullptr);

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

	newFrame();
	ImGui::NewFrame();

	int i = 0;
	for (auto& element : m_UIElements)
	{
		//if(i++)
		if (element->active())
		{
			element->draw();
		}
	}

	ImGui::Render();

	GUI::drawRenderData(*device, imgui, ImGui::GetDrawData(), cmdBuffer);
}

bool UI::mouseOverlap() const
{
	return ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered();
}

vkh::structs::Image* UI::loadImage(std::string path)
{
	// already loaded
	if (auto findIt = m_loadedImages.find(path); findIt != m_loadedImages.end())
		return &findIt->second;

	int width, height, channels;
	auto data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!data)
		throw std::runtime_error("Unknown image " + path);

	vkh::structs::Image newImage;
	device->createImage(
		width, height,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		newImage
	);
	// uploadBuffer
	auto cmdBuf = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	vkh::structs::Buffer& stagingBuffer = device->getStagingBuffer(width * height * channels * sizeof(unsigned char), data);

	newImage.transitionLayout(*device, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuf);

	VkBufferImageCopy region{};
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(cmdBuf, stagingBuffer, newImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	newImage.transitionLayout(*device, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdBuf);
	device->flushCommandBuffer(cmdBuf, device->queues.graphics);
	device->freeStagingBuffer(stagingBuffer);
	
	m_loadedImages[path] = newImage;
	return &m_loadedImages[path];
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

void UI::newFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() &&
		"Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	{
		auto windowSize = App::window.getWindowSize();
		io.DisplaySize = ImVec2(windowSize.width, windowSize.height);

		auto framebufferSize = App::window.getFramebufferSize();
		if (windowSize.width > 0 && windowSize.height > 0)
		{
			io.DisplayFramebufferScale = ImVec2(float(framebufferSize.width) / framebufferSize.width, float(framebufferSize.height) / framebufferSize.height);
		}

	} 
	{
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i)
		{
			io.MouseDown[i] = GUI::ImGuiInfo::mouseJustPressed[i] || App::input.mouse.pressedButton(i);
			GUI::ImGuiInfo::mouseJustPressed[i] = false;
		}

		// updateMOusePOs
		const ImVec2 mousePosBackup = io.MousePos;
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

		const bool focused = App::window.isFocused();

		if (focused)
		{
			if (!io.WantSetMousePos)
			{
				auto cursorPos = App::window.getMousePosition();
				io.MousePos = ImVec2(cursorPos.x, cursorPos.y);
			}
		}
	}
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

