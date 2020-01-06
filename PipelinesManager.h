#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include <optional>

#include "vulkanHelper/VulkanStructs.h"
#include "VulkanInfo.h"
#include "VulkanData.h"
#include "GraphicsObjects.h"

class VulkanBase;
class PipelinesManager
{
	using PipCreateInfo = const Info::PipelineCreateInfo&;

	std::pair<std::vector<char>, std::vector<char>> loadSuitableShaders(const Info::VertexInfo& vertexInfo) const;
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
	inline VkPipelineLayout createPipelineLayout(const Info::Layouts& laoyoutsInfo);

	void processMeshData(VD::MeshData& meshData, const Info::DrawInfo& drawInfo);

	Info::PipelineCreateInfo generatePipelineCreateInfoFromMeshData(const VD::MeshData& meshData, const Info::DrawInfo& drawInfo) const;
	VD::SharedPipeline getPipeline(const Info::PipelineCreateInfo& pipelineCreateInfo);
	VD::SharedPipeline createPipeline(const Info::PipelineCreateInfo& pipelineCreateInfo);
	std::optional<VD::SharedPipeline> findPipeline(const Info::PipelineCreateInfo& pipelineCreateInfo);
	bool comparePipelineInfo(const Info::PipelineInfo& lhs, const Info::PipelineInfo& rhs);

	VulkanBase* vkBase;
	vkh::structs::VulkanDevice* device;

	std::vector<VD::SharedPipeline> pipelines;
public:

	void init(VulkanBase* vkBase);
	void cleanUp(const VkAllocationCallbacks* allocator);

	void processModelData(VD::ModelData& modelData, const Info::DrawInfo& drawInfo);
};

