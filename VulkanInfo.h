#pragma once
#include "vulkanHelper/VulkanStructs.h"
#include <variant>
#include <string>

class Model;
namespace Info
{
	// Model
	struct ModelInfo
	{
		std::variant<Model*, std::string> model;
	};
	// Set
	struct DescriptorBinding
	{
		uint32_t binding;
		VkShaderStageFlags stage;
		VkDescriptorType type;
	};
	using DescriptorBindings = std::vector<DescriptorBinding>;

	struct DescriptorPoolInfo
	{
		size_t maxSets;
		std::vector<VkDescriptorType> supportedTypes;
	};

	struct DescriptorSetLayoutInfo
	{
		Info::DescriptorBindings bindings;
		std::optional<const VkSampler*> sampler;
	};
	struct DescriptorSetInfo
	{
		Info::DescriptorBindings bindings;

		std::vector<const vkh::structs::Buffer*> dstBuffers;
		std::optional<const vkh::structs::Image*> srcImage;
		std::optional<const VkSampler*> sampler;
	};
	
	// Pipeline
	struct DrawInfo
	{
		VkPolygonMode polygon = {};
		VkPrimitiveTopology topology = {};
		double lineWidth = 1.0f;
	};
	struct VertexInfo
	{
		uint64_t vertexType;
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

	struct PipelineCreateInfo
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

	struct PipelineInfo
	{
		Info::VertexInfo vertexInfo;
		Info::DrawInfo drawInfo;
	};
}