#pragma once
#include "..\vulkanHelper\VulkanHelper.h"
#include "..\vulkanHelper\VulkanStructs.h"

#include <imgui/imgui.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace GUI
{
	struct ImGuiInfo
	{
		static GLFWcursor* cursors[ImGuiMouseCursor_COUNT];
		// 5 mouse buttonst
		static bool mouseJustPressed[5];


		VkSampler fontSampler;


		VkDescriptorSetLayout setLayout;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		vkh::structs::Image fontImage;

		vkh::structs::Buffer vertexBuffer;
		vkh::structs::Buffer indexBuffer;

		void cleanup(VkDevice device, const VkAllocationCallbacks* allocator);
	};

	bool createDeviceObjects(const vkh::structs::VulkanDevice& device, const VkRenderPass& renderPass, GUI::ImGuiInfo& imguiObject);
	bool initGlfwImplementation(GUI::ImGuiInfo& imguiObject);
	bool createFontsTexture(const vkh::structs::VulkanDevice& device, GUI::ImGuiInfo& imguiObject);
	void setupRenderState(const vkh::structs::VulkanDevice& device, GUI::ImGuiInfo& imgui,
		ImDrawData* drawData, VkCommandBuffer cmdBuffer, int fbWidth, int fbHeight);
	void drawRenderData(const vkh::structs::VulkanDevice& device, GUI::ImGuiInfo& imgui, ImDrawData* drawData, VkCommandBuffer cmdBuffer);

	void updateMousePosAndButtons(GLFWwindow* window);
	void updateMouseCursor(GLFWwindow* window);
	void newFrame(GLFWwindow* window, double deltaTime);
}



