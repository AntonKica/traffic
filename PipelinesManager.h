#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include <optional>

#include "vulkanHelper/VulkanStructs.h"
#include "GraphicsObjects.h"

namespace PipelineSettings
{

}

namespace Info
{
	struct DrawInfo
	{
		VkPolygonMode polygon = {};
		VkPrimitiveTopology topology = {};
		double lineWidth = 1.0f;
	};
	struct VertexInfo
	{
		GO::VertexType vertexType;
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributes;
	};
	struct ViewportInfo
	{
		//bool dynamicState = false;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
		VkExtent2D viewportExtent;
		VkExtent2D scissorsExtent;
	};
	struct MultisampleInfo
	{
		VkBool32 sampleShading = false;
		float minSampleShading = 0.0f;
		VkSampleCountFlagBits samples;
		const VkSampleMask* sampleMask;
	};
	struct ColorBlendingInfo
	{
		bool writeAllMasks = true;
		bool blendEnable = false;
		VkBlendOp blendOp;
	};
	struct DepthStencilInfo
	{
		bool enableDepth;
		float minDepth;
		float maxDepth;
		// stencil not atm
	};
	struct Layouts
	{
		std::vector<const VkPushConstantRange*> pushRanges;
		const VkDescriptorSetLayout* setLayout;
	};
	struct PipelineInfo
	{
		VertexInfo vertexInfo;
		ViewportInfo viewportInfo;
		DrawInfo drawInfo;
		MultisampleInfo multisample;
		ColorBlendingInfo colorBlending;
		DepthStencilInfo depthStencil;
		Layouts layouts;

		VkRenderPass renderPass;
	};

	struct PipelineReference
	{
		GO::ID pipeline;
		GO::ID pipelineLayout;
		DrawInfo draw;
		VertexInfo vertInfo;
	};
}

class VulkanBase;
class PipelinesManager
{
	using PipInfo = const Info::PipelineInfo&;

	VulkanBase* vkBase;
	vkh::structs::VulkanDevice* device;
	std::map<GO::ID, VkPipeline> pipelines;
	std::map<GO::ID, VkPipelineLayout> pipelineLayouts;
	std::map<GO::ID, Info::PipelineReference> pipelineReferences;

	std::pair<std::vector<char>, std::vector<char>> loadSuitableShaders(const Info::VertexInfo& vertexInfo) const;
	std::pair<GO::ID, GO::ID> creteGraphicsPipeline(const PipInfo& pipelineInfo);
	inline VkShaderModule createShaderModule(const std::vector<char>& shaderBuffer) const;
	inline VkPipelineVertexInputStateCreateInfo createVertexInputState(const Info::VertexInfo& pipelineInfo) const;
	inline VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState(const Info::DrawInfo& drawInfo) const;
	inline std::pair<VkViewport, VkRect2D>  createViewportAndScissors(const Info::ViewportInfo& viewportInfo) const;
	inline VkPipelineViewportStateCreateInfo createViewportState(const VkViewport* viewport, const VkRect2D* scissors) const;
	inline VkPipelineRasterizationStateCreateInfo createRasterizationState(const Info::DrawInfo& drawInfo) const;
	inline VkPipelineMultisampleStateCreateInfo createMultisampleState(const Info::MultisampleInfo& multisampleInfo) const;
	inline VkPipelineColorBlendAttachmentState createColorBlendAttachmentState(const Info::ColorBlendingInfo& colorBlendingInfo) const;
	inline VkPipelineColorBlendStateCreateInfo createColorBlendState(const VkPipelineColorBlendAttachmentState* colorBlendingAttachment) const;
	inline VkPipelineDepthStencilStateCreateInfo createDepthStencilState(const Info::DepthStencilInfo& depthStencilInfo) const;
	inline GO::ID createPipelineLayout(const Info::Layouts& laoyoutsInfo);

	std::optional<GO::ID> findPipeline(PipInfo pipelineIinfo) const;

	VkPipeline& getPipeline(GO::ID id);
	VkPipelineLayout& getPipelineLayout(GO::ID id);
public:

	void init(VulkanBase* vkBase);
	void cleanup(const VkAllocationCallbacks* allocator);
	GO::ID getPipelineReference(const PipInfo& pipelineInfo);
	VkPipeline& getPipelineFromReference(GO::ID referenceID);
	VkPipelineLayout& getPipelineLayoutFromReference(GO::ID referenceID);
};

