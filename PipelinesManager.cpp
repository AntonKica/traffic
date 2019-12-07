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
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	// YAYAY
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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

inline VkPipelineLayout PipelinesManager::createPipelineLayout(
	const Info::Layouts& laoyoutsInfo)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo =
		vkh::initializers::pipelineLayoutCreateInfo(
			laoyoutsInfo.pushRanges.size(), laoyoutsInfo.pushRanges[0],
			1, laoyoutsInfo.setLayout
		);

	VkPipelineLayout layout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &layout));



	return layout;
}
/*
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
}*/


void PipelinesManager::init(VulkanBase* vkBase)
{
	this->vkBase = vkBase;
	this->device = vkBase->getDevice();
}

void PipelinesManager::cleanup(const VkAllocationCallbacks* allocator)
{
	for (auto& pipeline : pipelines)
	{
		vkDestroyPipelineLayout(*device, pipeline->pipelineLayout, allocator);
		vkDestroyPipeline(*device, pipeline->pipeline, allocator);
	}
}

void PipelinesManager::processModelData(VD::ModelData& modelData, const Info::DrawInfo& drawInfo)
{
	for (auto& meshData : modelData.meshDatas)
	{
		processMeshData(meshData, drawInfo);
	}
}

std::pair<std::vector<char>, std::vector<char>> PipelinesManager::loadSuitableShaders(const Info::VertexInfo& vertexInfo) const
{
	constexpr std::array<const char*, 6> shaderPaths =
	{
			"shaders/basicShaderVert.spv ", "shaders/basicShaderFrag.spv",
			"shaders/coloredShaderVert.spv", "shaders/coloredShaderFrag.spv",
			"shaders/texturedShaderVert.spv","shaders/texturedShaderFrag.spv"
	};

	auto vertexFlags = vertexInfo.vertexType;
	std::pair<int, int> indexes;
	if (vertexFlags & VD::VertexType::POSITION)
	{
		auto idk = (vertexFlags & VD::VertexType::POSITION);
		if (vertexFlags & VD::VertexType::POSITION)
			indexes = { 0,1 };
		if (vertexFlags & VD::VertexType::COLOR)
			indexes = { 2,3 };
		else if (vertexFlags & VD::VertexType::TEXTURE)
			indexes = { 4,5 };
	}
	else
	{
		throw std::runtime_error("Unknown vertex flags/type");
	}

	std::vector<char> vertexBuffer = loadFileToBuffer(shaderPaths[indexes.first]);
	std::vector<char> fragmnentBuufer = loadFileToBuffer(shaderPaths[indexes.second]);

	return std::make_pair(vertexBuffer, fragmnentBuufer);
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

void PipelinesManager::processMeshData(VD::MeshData& meshData, const Info::DrawInfo& drawInfo)
{
	auto pipelineCreateInfo = generatePipelineCreateInfoFromMeshData(meshData, drawInfo);

	meshData.pipeline = getPipeline(pipelineCreateInfo);
}

Info::PipelineCreateInfo PipelinesManager::generatePipelineCreateInfoFromMeshData(const VD::MeshData& meshData, const Info::DrawInfo& drawInfo) const
{
	using namespace Info;

	VertexInfo vertexInfo;
	vertexInfo.vertexType = meshData.drawData.vertices->buffer.type;
	vertexInfo.attributes = VD::getAttributeDescriptionsFromFlags(vertexInfo.vertexType);
	vertexInfo.bindingDescription = VD::getBindingDescriptionFromFlags(vertexInfo.vertexType);

	auto swapChain = vkBase->getSwapchain();
	ViewportInfo viewportInfo;
	viewportInfo.minDepth = 0.0f;
	viewportInfo.maxDepth = 1.0f;
	viewportInfo.viewportExtent = swapChain.extent;
	viewportInfo.scissorsExtent = swapChain.extent;
	//drawinfo
	MultisampleInfo multisampleInfo;
	multisampleInfo.sampleShading = false;
	multisampleInfo.minSampleShading = 0.0f;
	multisampleInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	multisampleInfo.sampleMask = nullptr;

	ColorBlendingInfo colorBlendingInfo;
#pragma message ("Pimp up blending!")
	colorBlendingInfo.blendEnable = true;
	colorBlendingInfo.writeAllMasks = true;
	colorBlendingInfo.blendOp = VK_BLEND_OP_ADD;

	DepthStencilInfo depthStencilInfo;
	depthStencilInfo.enableDepth = true;
	depthStencilInfo.minDepth = 0.0f;
	depthStencilInfo.maxDepth = 1.0f;

	Layouts layouts;
	layouts.pushRanges = { &vkBase->getPushRange() };
	layouts.setLayout = &meshData.descriptorSet->setLayout->layout;

	Info::PipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.vertexInfo = vertexInfo;
	pipelineCreateInfo.viewportInfo = viewportInfo;
	pipelineCreateInfo.drawInfo = drawInfo;
	pipelineCreateInfo.multisample = multisampleInfo;
	pipelineCreateInfo.colorBlending = colorBlendingInfo;
	pipelineCreateInfo.depthStencil = depthStencilInfo;
	pipelineCreateInfo.layouts = layouts;
	pipelineCreateInfo.renderPass = vkBase->getRenderPass();

	return pipelineCreateInfo;
}

VD::SharedPipeline PipelinesManager::getPipeline(const Info::PipelineCreateInfo& pipelineCreateInfo)
{
	auto possiblePipeline = findPipeline(pipelineCreateInfo);

	if (possiblePipeline)
		return possiblePipeline.value();
	else
		return createPipeline(pipelineCreateInfo);
}

VD::SharedPipeline PipelinesManager::createPipeline(const Info::PipelineCreateInfo& pipelineCreateInfo)
{
	auto [vertShaderCode, fragShaderCode] = loadSuitableShaders(pipelineCreateInfo.vertexInfo);

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

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = createInputAssemblyState(pipelineCreateInfo.drawInfo);
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = createVertexInputState(pipelineCreateInfo.vertexInfo);

	const auto [viewport, scissors] = createViewportAndScissors(pipelineCreateInfo.viewportInfo);
	VkPipelineViewportStateCreateInfo viewportInfo = createViewportState(&viewport, &scissors);
	VkPipelineRasterizationStateCreateInfo rasterizer = createRasterizationState(pipelineCreateInfo.drawInfo);

	VkPipelineMultisampleStateCreateInfo multisampling = createMultisampleState(pipelineCreateInfo.multisample);

	VkPipelineColorBlendAttachmentState colorBlendingAttachment = createColorBlendAttachmentState(pipelineCreateInfo.colorBlending);
	VkPipelineColorBlendStateCreateInfo colorBlending = createColorBlendState(&colorBlendingAttachment);

	VkPipelineDepthStencilStateCreateInfo depthStencil = createDepthStencilState(pipelineCreateInfo.depthStencil);

	auto newPipelineLayout = createPipelineLayout(pipelineCreateInfo.layouts);

	VkGraphicsPipelineCreateInfo createInfo = vkh::initializers::graphicsPipelineCreateInfo();
	createInfo.stageCount = shaderStages.size();
	createInfo.pStages = shaderStages.data();
	createInfo.pVertexInputState = &vertexInputInfo;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pViewportState = &viewportInfo;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &multisampling;
	createInfo.pDepthStencilState = &depthStencil;
	createInfo.pColorBlendState = &colorBlending;
	createInfo.pDynamicState = nullptr;
	createInfo.layout = newPipelineLayout;
	createInfo.renderPass = pipelineCreateInfo.renderPass;
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = -1;

	VkPipeline newPipeline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &newPipeline));

	vkDestroyShaderModule(*device, moduleVert, nullptr);
	vkDestroyShaderModule(*device, moduleFrag, nullptr);

	Info::PipelineInfo pipInfo;
	pipInfo.drawInfo = pipelineCreateInfo.drawInfo;
	pipInfo.vertexInfo = pipelineCreateInfo.vertexInfo;

	VD::Pipeline pipeline;
	pipeline.info = pipInfo;
	pipeline.pipeline = newPipeline;
	pipeline.pipelineLayout = newPipelineLayout;

	pipelines.push_back(std::make_shared<VD::Pipeline>(pipeline));

	return pipelines.back();
}

std::optional<VD::SharedPipeline> PipelinesManager::findPipeline(const Info::PipelineCreateInfo& pipelineCreateInfo)
{
	std::optional<VD::SharedPipeline> pipeline;

	for (const auto& _pipeline : pipelines)
	{
		Info::PipelineInfo pipInfo;
		pipInfo.drawInfo = pipelineCreateInfo.drawInfo;
		pipInfo.vertexInfo = pipelineCreateInfo.vertexInfo;

		if (comparePipelineInfo(_pipeline->info, pipInfo))
		{
			pipeline = _pipeline;
			break;
		}
	}

	return pipeline;
}

template<class ptrT > bool sameData(ptrT* first, ptrT* second, size_t size)
{
	return std::memcmp(first, second, size) == 0;
}

bool PipelinesManager::comparePipelineInfo(const Info::PipelineInfo& lhs, const Info::PipelineInfo& rhs)
{
	// apparently in one statement it doesnt work
	bool sameDrawInfo = lhs.drawInfo.lineWidth == rhs.drawInfo.lineWidth &&
		lhs.drawInfo.polygon == rhs.drawInfo.polygon &&
		lhs.drawInfo.topology == rhs.drawInfo.topology;

	bool sameVertexInfo = false;
	if (lhs.vertexInfo.vertexType == rhs.vertexInfo.vertexType)
	{
		const auto& lhsBds = lhs.vertexInfo.bindingDescription;
		const auto& rhsBds = rhs.vertexInfo.bindingDescription;
		if (lhsBds.binding == rhsBds.binding &&
			lhsBds.inputRate == rhsBds.inputRate &&
			lhsBds.stride == rhsBds.stride)
		{
			const auto& lhsAtbs = lhs.vertexInfo.attributes;
			const auto& rhsAtbs = rhs.vertexInfo.attributes;
			for (const auto& la : lhsAtbs)
			{
				for (const auto& ra : rhsAtbs)
				{
					if (la.binding == ra.binding &&
						la.format == ra.format &&
						la.location == ra.location &&
						la.offset == ra.offset)
					{
						sameVertexInfo = true;

						return sameDrawInfo && sameVertexInfo;
					}
				}
			}
		}
	}
	return sameDrawInfo && sameVertexInfo;

}
