#pragma once
#include <GLFW/glfw3.h>

#include "imgui/ImguiVulkanImplementation.h"
#include "vulkanHelper/VulkanStructs.h"

class VulkanBase;

class UI
{
private:
	VulkanBase* vulkanBase;
	vkh::structs::VulkanDevice* device;
	GLFWwindow* window;

	GUI::ImGuiInfo imgui;
	bool hidden;
	
	std::string selectedName;
	int selectedNum;
public:
	void initUI(VulkanBase* pVulkanBase);
	void destroyUI();

	void drawUI(VkCommandBuffer cmdBuffer);

	int getSelection() const;
	bool mouseOverlap() const;
private:
	void createResources();


	// or what is it called
	void createBoxes();

};