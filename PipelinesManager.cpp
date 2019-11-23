#include "PipelinesManager.h"

#include "VulkanBase.h"
#include "vulkanHelper/VulkanHelper.h"
#include "Utilities.h"
using namespace Utility;

#include <vector>
#include <fstream>
//#include <>

static std::vector<char> loadFileToBuffer(const std::string& filePath)
{
	std::ifstream file(filePath.c_str(), std::ifstream::ate | std::ifstream::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file " + filePath);

	size_t fileSize = file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	return buffer;
}

VkPipelineVertexInputStateCreateInfo PipelinesManager::createVertexInputState(
	const Info::VertexInfo& vertInfo) const
{
	VkPipelineVertexInputStateCreateInfo createInfo =
		vkh::initializers::pipelineVertexInputStateCreateInfo(
			1, 
			&vertInfo.bindingDescription,
			vertInfo.attributes.size(), 
			vertInfo.attributes.data()
		);

	return createInfo;
}

VkPipelineInputAssemblyStateCreateInfo PipelinesManager::createInputAssemblyState(
	const Info::DrawInfo& drawInfo) const
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = drawInfo.topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	return inputAssembly;
}

std::pair<VkViewport, VkRect2D> PipelinesManager::createViewportAndScissors(
	const Info::ViewportInfo& viewportInfo) const
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = viewportInfo.viewportExtent.width;
	viewport.height = viewportInfo.viewportExtent.height;
	viewport.minDepth = viewportInfo.minDepth;
	viewport.maxDepth = viewportInfo.maxDepth;

	VkRect2D scissors{};
	scissors.offset = { 0, 0 };
	scissors.extent = viewportInfo.scissorsExtent;

	return std::make_pair(viewport, scissors);
}

VkPipelineViewportStateCreateInfo PipelinesManager::createViewportState(
	const VkViewport* viewport,
	const VkRect2D* scissors) const
{
	VkPipelineViewportStateCreateInfo createInfo = vkh::initializers::pipelineViewportStateCreateInfo(
		1, scissors,
		1, viewport
	);

	return createInfo;
}

VkPipelineRasterizationStateCreateInfo PipelinesManager::createRasterizationState(
	const Info::DrawInfo& drawInfo) const
{
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = drawInfo.polygon;
	rasterizer.lineWidth = drawInfo.lineWidth;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	return rasterizer;
}

inline VkPipelineMultisampleStateCreateInfo PipelinesManager::createMultisampleState(
	const Info::MultisampleInfo& multisampleInfo) const
{
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = multisampleInfo.sampleShading;
	multisampling.rasterizationSamples = multisampleInfo.samples;
	multisampling.minSampleShading = multisampleInfo.minSampleShading;
	multisampling.pSampleMask = multisampleInfo.sampleMask;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	return multisampling;
}

inline VkPipelineColorBlendAttachmentState PipelinesManager::createColorBlendAttachmentState(
	const Info::ColorBlendingInfo& colorBlendingInfo) const
{
	VkPipelineColorBlendAttachmentState colorBlendingAttachment{};
	colorBlendingAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendingAttachment.blendEnable = colorBlendingInfo.blendEnable;

	if (colorBlendingInfo.blendEnable)
	{
		colorBlendingAttachment.colorBlendOp = colorBlendingInfo.blendOp;
		colorBlendingAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendingAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

		colorBlendingAttachment.alphaBlendOp = colorBlendingInfo.blendOp;
		colorBlendingAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendingAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	}

	return colorBlendingAttachment;
}

inline VkPipelineColorBlendStateCreateInfo PipelinesManager::createColorBlendState(
	const VkPipelineColorBlendAttachmentState* colorBlendingAttachment) const
{
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = colorBlendingAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	return colorBlending;
}

inline VkPipelineDepthStencilStateCreateInfo PipelinesManager::createDepthStencilState(
	const Info::DepthStencilInfo& depthStencilInfo) const
{
	VkPipelineDepthStencilStateCreateInfo depthStencil = vkh::initializers::pipelineDepthStencilStateCreateInfo();
	depthStencil.depthTestEnable = depthStencilInfo.enableDepth;
	depthStencil.depthWriteEnable = depthStencilInfo.enableDepth;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = depthStencilInfo.minDepth;
	depthStencil.maxDepthBounds = depthStencilInfo.maxDepth;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	return depthStencil;
}

inline GO::ID PipelinesManager::createPipelineLayout(
	const Info::Layouts& laoyoutsInfo)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo =
		vkh::initializers::pipelineLayoutCreateInfo(
			laoyoutsInfo.pushRanges.size(), laoyoutsInfo.pushRanges[0],
			1, laoyoutsInfo.setLayout
		);

	VkPipelineLayout layout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &layout));

	GO::ID layoutID = generateNextContainerID(pipelineLayouts);
	pipelineLayouts[layoutID] = layout;

	return layoutID;
}

std::optional<GO::ID> PipelinesManager::findPipeline(PipInfo pipelineIinfo) const
{
	std::optional<GO::ID> pipId;

	auto sameData = [](auto* lhs, auto* rhs, size_t size)
	{	return std::memcmp(lhs, rhs, size) == 0; };

	for (const auto& [id, pipRef] : pipelineReferences)
	{

		if (sameData(&pipRef.draw, &pipelineIinfo.drawInfo, sizeof(pipRef.draw)) &&
			sameData(&pipRef.vertInfo, &pipelineIinfo.vertexInfo, sizeof(pipRef.draw)))
		{
			pipId = id;
			break;
		}
	}

	return pipId;
}


void PipelinesManager::init(VulkanBase* vkBase)
{
	this->vkBase = vkBase;
	this->device = vkBase->getDevice();
}

void PipelinesManager::cleanup(const VkAllocationCallbacks* allocator)
{
	for (auto& [id, pipeline] : pipelines)
		vkDestroyPipeline(*device, pipeline, allocator);
	for (auto& [id, pipelineLayout] : pipelineLayouts)
		vkDestroyPipelineLayout(*device, pipelineLayout, allocator);
}

GO::ID PipelinesManager::getPipelineReference(const PipInfo& pipelineInfo)
{
	static int i = 0;
	if (i == 1)
		std::cout << '\n';
	++i;

	std::optional<GO::ID> pipId = findPipeline(pipelineInfo);
	if (pipId)
		return pipId.value();

	const auto& [setID, pipID] = creteGraphicsPipeline(pipelineInfo);
	
	Info::PipelineReference pipRef;
	pipRef.vertInfo = pipelineInfo.vertexInfo;
	pipRef.draw = pipelineInfo.drawInfo;
	pipRef.pipelineLayout = setID;
	pipRef.pipeline = pipID;

	GO::ID refID = generateNextContainerID(pipelineReferences);
	pipelineReferences[refID] = pipRef;

	return refID;
}

VkPipeline& PipelinesManager::getPipelineFromReference(GO::ID referenceID)
{
	return getPipeline(pipelineReferences[referenceID].pipeline);
}

VkPipelineLayout& PipelinesManager::getPipelineLayoutFromReference(GO::ID referenceID)
{
	return getPipelineLayout(pipelineReferences[referenceID].pipelineLayout);
}

std::pair<std::vector<char>, std::vector<char>> PipelinesManager::loadSuitableShaders(const Info::VertexInfo& vertexInfo) const
{
	constexpr std::array<const char*, 6> shaderPaths =
	{
			"shaders/basicShaderVert.spv ", "shaders/basicShaderFrag.spv",
			"shaders/coloredShaderVert.spv", "shaders/coloredShaderFrag.spv",
			"shaders/texturedShaderVert.spv","shaders/texturedShaderFrag.spv"
	};

	std::pair<int, int> indexes;
	switch (vertexInfo.vertexType)
	{
	case GraphicsObjects::DEFAULT:
		indexes = { 0,1 };
		break;
	case GraphicsObjects::COLORED:
		indexes = { 2,3 };
		break;
	case GraphicsObjects::TEXTURED:
		indexes = { 4,5 };
		break;
	default:
		throw std::runtime_error("Unknown vertex type!");
	}


	std::vector<char> vertexBuffer = loadFileToBuffer(shaderPaths[indexes.first]);
	std::vector<char> indexBuffer = loadFileToBuffer(shaderPaths[indexes.second]);

	return std::make_pair(vertexBuffer, indexBuffer);
}

std::pair<GO::ID, GO::ID> PipelinesManager::creteGraphicsPipeline(const PipInfo& pipelineInfo)
{
	auto [vertShaderCode, fragShaderCode] = loadSuitableShaders(pipelineInfo.vertexInfo);

	auto moduleVert = createShaderModule(vertShaderCode);
	auto moduleFrag = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertStageInfo =
		vkh::initializers::pipelineShaderStageCreateInfo(
			VK_SHADER_STAGE_VERTEX_BIT,
			moduleVert
		);

	VkPipelineShaderStageCreateInfo fragStageInfo =
		vkh::initializers::pipelineShaderStageCreateInfo(
			VK_SHADER_STAGE_FRAGMENT_BIT,
			moduleFrag
		);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{ fragStageInfo, vertStageInfo };

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = createInputAssemblyState(pipelineInfo.drawInfo);
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = createVertexInputState(pipelineInfo.vertexInfo);

	const auto[viewport, scissors] = createViewportAndScissors(pipelineInfo.viewportInfo);
	VkPipelineViewportStateCreateInfo viewportInfo = createViewportState(&viewport, &scissors);
	VkPipelineRasterizationStateCreateInfo rasterizer = createRasterizationState(pipelineInfo.drawInfo);

	VkPipelineMultisampleStateCreateInfo multisampling = createMultisampleState(pipelineInfo.multisample);

	VkPipelineColorBlendAttachmentState colorBlendingAttachment = createColorBlendAttachmentState(pipelineInfo.colorBlending);
	VkPipelineColorBlendStateCreateInfo colorBlending = createColorBlendState(&colorBlendingAttachment);

	VkPipelineDepthStencilStateCreateInfo depthStencil = createDepthStencilState(pipelineInfo.depthStencil);

	GO::ID pipelineLayoutID = createPipelineLayout(pipelineInfo.layouts);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = vkh::initializers::graphicsPipelineCreateInfo();
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthStencil;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = getPipelineLayout(pipelineLayoutID);
	pipelineCreateInfo.renderPass = pipelineInfo.renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	VkPipeline newPipeline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &newPipeline));

	vkDestroyShaderModule(*device, moduleVert, nullptr);
	vkDestroyShaderModule(*device, moduleFrag, nullptr);

	GO::ID pipelineID = generateNextContainerID(pipelines);
	pipelines[pipelineID] = newPipeline;

	return std::make_pair(pipelineLayoutID, pipelineID);
}

inline VkShaderModule PipelinesManager::createShaderModule(const std::vector<char>& shaderBuffer) const
{
	VkShaderModuleCreateInfo moduleInfo = vkh::initializers::shaderModuleCreateInfo(
		shaderBuffer.size(),
		reinterpret_cast<const uint32_t*>(shaderBuffer.data())
	);

	VkShaderModule shaderModule;
	VK_CHECK_RESULT(vkCreateShaderModule(*device, &moduleInfo, nullptr, &shaderModule));
	//load code
	return shaderModule;
}
VkPipeline& PipelinesManager::getPipeline(GO::ID id)
{
	auto pipelineIt = pipelines.find(id);
	if (pipelineIt == std::end(pipelines))
		throw std::runtime_error("Unknmown pipeline " + std::to_string(id));

	return pipelineIt->second;
}

VkPipelineLayout& PipelinesManager::getPipelineLayout(GO::ID id)
{
	auto pipLayoutIt = pipelineLayouts.find(id);
	if (pipLayoutIt == std::end(pipelineLayouts))
		throw std::runtime_error("Unknmown pipeline " + std::to_string(id));

	return pipLayoutIt->second;
}
