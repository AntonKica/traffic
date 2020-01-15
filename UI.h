#pragma once
#include <GLFW/glfw3.h>
#include <map>
#include "imgui/ImguiVulkanImplementation.h"
#include "vulkanHelper/VulkanStructs.h"

class VulkanBase;


class UIElement
{
public:
	UIElement();
	~UIElement();
	UIElement(const UIElement& otherElement);
	UIElement(UIElement&& otherElement);
	UIElement& operator=(const UIElement& otherElement);
	UIElement& operator=(UIElement&& otherElement);

	virtual void draw() = 0;

	bool active() const;
	void setActive(bool active);
private:
	bool m_active = false;
};

class UI
{
private:
	VulkanBase* vulkanBase;
	vkh::structs::VulkanDevice* device;

	std::vector<UIElement*> m_UIElements;
	std::map<std::string, vkh::structs::Image> m_loadedImages;

	GUI::ImGuiInfo imgui;
public:
	static UI& getInstance();

	void initUI(VulkanBase* pVulkanBase);
	void destroyUI();

	void drawUI(VkCommandBuffer cmdBuffer);

	bool mouseOverlap() const;

	vkh::structs::Image* loadImage(std::string path);
private:

	UI();
	void createResources();

	void newFrame();

	friend class UIElement;
	void registerUIElement(UIElement* element);
	void unregisterUIElement(UIElement* element);

	// or what is it called
};
