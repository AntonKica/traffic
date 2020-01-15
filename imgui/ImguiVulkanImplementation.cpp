#include "ImguiVulkanImplementation.h"
#include <vulkan/vulkan.h>
#include "..\vulkanHelper\VulkanHelper.h"
#include "..\vulkanHelper\VulkanStructs.h"

#include <imgui/imgui.h>

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
	Out.Color = aColor;
	Out.UV = aUV;
	gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static uint32_t __glsl_shader_vert_spv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
	0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
	0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
	0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
	0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
	0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
	0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
	0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
	0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
	0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
	0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
	0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
	0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
	0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
	0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
	0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
	0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
	0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
	0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
	0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
	0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
	0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
	0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
	0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
	0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
	0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
	0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
	0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
	0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
	0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
	0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
	0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
	0x0000002d,0x0000002c,0x000100fd,0x00010038
};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
	fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static uint32_t __glsl_shader_frag_spv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
	0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
	0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
	0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
	0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
	0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
	0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
	0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
	0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
	0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
	0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
	0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
	0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
	0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
	0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
	0x00010038
};

// initilizations
GLFWcursor* GUI::ImGuiInfo::cursors[] = { nullptr };
bool GUI::ImGuiInfo::mouseJustPressed[] = { false };
// 5 mouse buttonst

bool GUI::createDeviceObjects(
	const vkh::structs::VulkanDevice& device,
	const VkRenderPass& renderPass,
	GUI::ImGuiInfo& imguiObject)
{
	// createModules
	VkShaderModule shaderModules[2]{};
	{
		VkShaderModuleCreateInfo vertInfo = {};
		vertInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertInfo.codeSize = sizeof(__glsl_shader_vert_spv);
		vertInfo.pCode = (uint32_t*)__glsl_shader_vert_spv;
		VK_CHECK_RESULT(vkCreateShaderModule(device, &vertInfo, nullptr, &shaderModules[0]));

		VkShaderModuleCreateInfo fragInfo = {};
		fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragInfo.codeSize = sizeof(__glsl_shader_frag_spv);
		fragInfo.pCode = (uint32_t*)__glsl_shader_frag_spv;
		VK_CHECK_RESULT(vkCreateShaderModule(device, &fragInfo, nullptr, &shaderModules[1]));
	}

	// sampler
	{
		VkSamplerCreateInfo samplerInfo = vkh::initializers::samplerCreateInfo(
			VK_FILTER_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_MIPMAP_MODE_LINEAR);
		samplerInfo.minLod = -1000;
		samplerInfo.maxLod = 1000;
		VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &imguiObject.fontSampler));
	}

	// descriptors
	{
		std::vector<VkDescriptorSetLayoutBinding> binding =
		{
			vkh::initializers::descritptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				&imguiObject.fontSampler
				)
		};
		// laoyut
		VkDescriptorSetLayoutCreateInfo layoutInfo =
			vkh::initializers::descriptorSetLayoutCreateInfo(binding.size(), binding.data());
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &imguiObject.setLayout));

		// pool sizes and info
		VkDescriptorPoolSize poolSize =
			vkh::initializers::descriptorPoolSize(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		VkDescriptorPoolCreateInfo poolInfo = vkh::initializers::descriptorPoolCreateInfo(1, &poolSize, 1000);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiObject.descriptorPool));

		// desciptor set
		VkDescriptorSetAllocateInfo allocInfo =
			vkh::initializers::descriptorSetAllocateInfo(imguiObject.descriptorPool, 1, &imguiObject.setLayout);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &imguiObject.descriptorSet));
	}

	// not mine
	// pipeline layout
	{
		VkPushConstantRange push_constants[1] = {};
		push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		push_constants[0].offset = sizeof(float) * 0;
		push_constants[0].size = sizeof(float) * 4;
		VkDescriptorSetLayout set_layout[1] = { imguiObject.setLayout };
		VkPipelineLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_info.setLayoutCount = 1;
		layout_info.pSetLayouts = set_layout;
		layout_info.pushConstantRangeCount = 1;
		layout_info.pPushConstantRanges = push_constants;
		vkCreatePipelineLayout(device, &layout_info, nullptr, &imguiObject.pipelineLayout);
	}

	VkPipelineShaderStageCreateInfo stage[2] = {};
	stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stage[0].module = shaderModules[0];
	stage[0].pName = "main";
	stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stage[1].module = shaderModules[1];
	stage[1].pName = "main";

	//
	VkVertexInputBindingDescription binding_desc[1] = {};
	binding_desc[0].stride = sizeof(ImDrawVert);
	binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attribute_desc[3] = {};
	attribute_desc[0].location = 0;
	attribute_desc[0].binding = binding_desc[0].binding;
	attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_desc[0].offset = IM_OFFSETOF(ImDrawVert, pos);
	attribute_desc[1].location = 1;
	attribute_desc[1].binding = binding_desc[0].binding;
	attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_desc[1].offset = IM_OFFSETOF(ImDrawVert, uv);
	attribute_desc[2].location = 2;
	attribute_desc[2].binding = binding_desc[0].binding;
	attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
	attribute_desc[2].offset = IM_OFFSETOF(ImDrawVert, col);

	VkPipelineVertexInputStateCreateInfo vertex_info = {};
	vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_info.vertexBindingDescriptionCount = 1;
	vertex_info.pVertexBindingDescriptions = binding_desc;
	vertex_info.vertexAttributeDescriptionCount = 3;
	vertex_info.pVertexAttributeDescriptions = attribute_desc;

	//
	VkPipelineInputAssemblyStateCreateInfo ia_info = {};
	ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo raster_info = {};
	raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_info.polygonMode = VK_POLYGON_MODE_FILL;
	raster_info.cullMode = VK_CULL_MODE_NONE;
	raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	raster_info.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo ms_info = {};
	ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_attachment[1] = {};
	color_attachment[0].blendEnable = VK_TRUE;
	color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
	color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
	color_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo blend_info = {};
	blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend_info.attachmentCount = 1;
	blend_info.pAttachments = color_attachment;

	VkPipelineDepthStencilStateCreateInfo depth_info = {};
	depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	const int dynamicStateCount = 2;
	VkDynamicState dynamic_states[dynamicStateCount]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = (uint32_t)dynamicStateCount;
	dynamic_state.pDynamicStates = dynamic_states;

	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.flags = 0;
	info.stageCount = 2;
	info.pStages = stage;
	info.pVertexInputState = &vertex_info;
	info.pInputAssemblyState = &ia_info;
	info.pViewportState = &viewport_info;
	info.pRasterizationState = &raster_info;
	info.pMultisampleState = &ms_info;
	info.pDepthStencilState = &depth_info;
	info.pColorBlendState = &blend_info;
	info.pDynamicState = &dynamic_state;
	info.layout = imguiObject.pipelineLayout;
	info.renderPass = renderPass;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
		1, &info, nullptr, &imguiObject.graphicsPipeline));

	vkDestroyShaderModule(device, shaderModules[0], nullptr);
	vkDestroyShaderModule(device, shaderModules[1], nullptr);

	return true;
}

bool GUI::initGlfwImplementation(GUI::ImGuiInfo& imguiObject)
{
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.WantCaptureMouse = true;

	io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;

	// clipboard

#if defined(_WIN32)
	//io.ImeWindowHandle = (void*)glfwGetWin32();
#endif
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
	GUI::ImGuiInfo::cursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

	return true;
}

bool GUI::createFontsTexture(
	const vkh::structs::VulkanDevice& device,
	GUI::ImGuiInfo& imguiObject)
{
	ImGuiIO& io = ImGui::GetIO();

	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	size_t uploadSize = width * height * 4 * sizeof(char);
	// create fontImage and view
	device.createImage(
		width, height,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		imguiObject.fontImage
	);
	// uploadBuffer
	vkh::structs::Buffer uploadBuffer;
	device.createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		uploadSize, uploadBuffer, pixels);

	VkCommandBuffer copyBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	imguiObject.fontImage.transitionLayout(device, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyBuffer);

	VkBufferImageCopy region{};
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(copyBuffer, uploadBuffer, imguiObject.fontImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	imguiObject.fontImage.transitionLayout(device, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, copyBuffer);
	device.flushCommandBuffer(copyBuffer, device.queues.graphics);

	io.Fonts->TexID = (ImTextureID)(intptr_t)imguiObject.fontImage.image;

	uploadBuffer.cleanUp(device, nullptr);

	// update descriptor
	imguiObject.fontImage.setupDescriptor(imguiObject.fontSampler);
	VkWriteDescriptorSet write = vkh::initializers::writeDescriptorSet();
	write.descriptorCount = 1;
	write.dstSet = imguiObject.descriptorSet;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &imguiObject.fontImage.info;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);


	return true;
}

void GUI::setupRenderState(
	const vkh::structs::VulkanDevice& device,
	GUI::ImGuiInfo& imgui,
	ImDrawData* drawData,
	VkCommandBuffer cmdBuffer,
	int fbWidth,
	int fbHeight)
{
	//  pipeline
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, imgui.graphicsPipeline);
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, imgui.pipelineLayout,
		0, 1, &imgui.descriptorSet, 0, nullptr);

	// draw data
	VkBuffer vertexBuffers[] = { imgui.vertexBuffer };
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, imgui.indexBuffer, 0,
		sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = fbWidth;
	viewport.height = fbHeight;
	viewport.minDepth = 0.0;
	viewport.maxDepth = 1.0;

	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	// setup scale and translation
	float scale[2];
	scale[0] = 2.0f / drawData->DisplaySize.x;
	scale[1] = 2.0f / drawData->DisplaySize.y;
	float translate[2];
	translate[0] = -1.0f - drawData->DisplayPos.x * scale[0];
	translate[1] = -1.0f - drawData->DisplayPos.y * scale[1];
	vkCmdPushConstants(cmdBuffer, imgui.pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
	vkCmdPushConstants(cmdBuffer, imgui.pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
}

void GUI::drawRenderData(
	const vkh::structs::VulkanDevice& device,
	GUI::ImGuiInfo& imgui,
	ImDrawData* drawData,
	VkCommandBuffer cmdBuffer)
{
	int fbWidth = drawData->DisplaySize.x * drawData->FramebufferScale.x;
	int fbHeight = drawData->DisplaySize.y * drawData->FramebufferScale.y;
	// dont draw if min or nothing to draw
	if (fbWidth <= 0 || fbHeight <= 0 || drawData->TotalVtxCount == 0)
		return;


	size_t vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
	size_t indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

	if (vertexSize <= 0 || indexSize <= 0)
		return;

	// if small buffer, resize
	if (!imgui.vertexBuffer.initialized() || imgui.vertexBuffer.size < vertexSize)
	{
		vkDeviceWaitIdle(device);
		device.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexSize, imgui.vertexBuffer);
	}
	if (!imgui.indexBuffer.initialized() || imgui.indexBuffer.size < indexSize)
	{
		vkDeviceWaitIdle(device);
		device.createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexSize, imgui.indexBuffer);
	}

	// copy data
	imgui.vertexBuffer.map();
	imgui.indexBuffer.map();

	ImDrawVert* vertDst = reinterpret_cast<ImDrawVert*>(imgui.vertexBuffer.mapped);
	ImDrawIdx* idxDst = reinterpret_cast<ImDrawIdx*>(imgui.indexBuffer.mapped);

	for (int n = 0; n < drawData->CmdListsCount; ++n)
	{
		const ImDrawList* cmdList = drawData->CmdLists[n];
		memcpy(vertDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.size_in_bytes());
		memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.size_in_bytes());
		vertDst += cmdList->VtxBuffer.Size;
		idxDst += cmdList->IdxBuffer.Size;
	}
	imgui.vertexBuffer.unmap();
	imgui.indexBuffer.unmap();


	setupRenderState(device, imgui, drawData, cmdBuffer, fbWidth, fbHeight);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clipOff = drawData->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	int globalVtxOffset = 0;
	int globalIdxOffset = 0;
	for (int n = 0; n < drawData->CmdListsCount; ++n)
	{
		const ImDrawList* cmdList = drawData->CmdLists[n];
		for (int cmdIndex = 0; cmdIndex < cmdList->CmdBuffer.Size; ++cmdIndex)
		{
			const ImDrawCmd* pCmd = &cmdList->CmdBuffer[cmdIndex];
			if (pCmd->UserCallback != NULL)
			{
				if (pCmd->UserCallback == ImDrawCallback_ResetRenderState)
					setupRenderState(device, imgui, drawData, cmdBuffer, fbWidth, fbHeight);
				else
					pCmd->UserCallback(cmdList, pCmd);
			}
			else
			{
				ImVec4 clipRect;
				clipRect.x = (pCmd->ClipRect.x - clipOff.x) * clipScale.x;
				clipRect.y = (pCmd->ClipRect.y - clipOff.y) * clipScale.y;
				clipRect.z = (pCmd->ClipRect.z - clipOff.x) * clipScale.x;
				clipRect.w = (pCmd->ClipRect.w - clipOff.y) * clipScale.y;

				// if clipped
				if (clipRect.x < fbWidth && clipRect.y < fbHeight &&
					clipRect.x >= 0.0 && clipRect.y >= 0.0)
				{
					if (clipRect.x < 0.0f)
						clipRect.x = 0.0f;
					if (clipRect.y < 0.0f)
						clipRect.y = 0.0f;

					VkRect2D scissor{};
					scissor.offset.x = clipRect.x;
					scissor.offset.y = clipRect.y;
					scissor.extent = { uint32_t(clipRect.z - clipRect.x),  uint32_t(clipRect.w - clipRect.y) };

					vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

					vkCmdDrawIndexed(cmdBuffer, pCmd->ElemCount, 1,
						pCmd->IdxOffset + globalIdxOffset, pCmd->VtxOffset + globalVtxOffset, 0);
				}
			}
		}
		globalVtxOffset += cmdList->VtxBuffer.Size;
		globalIdxOffset += cmdList->IdxBuffer.Size;
	}
}

void GUI::updateMouseCursor(GLFWwindow* window)
{
	ImGuiIO& io = ImGui::GetIO();
	// checks
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) ||
		glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		return;
	}

	ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
	if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else
	{
		glfwSetCursor(window, GUI::ImGuiInfo::cursors[imguiCursor] ? GUI::ImGuiInfo::cursors[imguiCursor] :
			GUI::ImGuiInfo::cursors[ImGuiMouseCursor_Arrow]);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void GUI::ImGuiInfo::cleanup(VkDevice device, const VkAllocationCallbacks* allocator)
{
	vkDestroyPipeline(device, graphicsPipeline, allocator);
	vkDestroyPipelineLayout(device, pipelineLayout, allocator);
	vkDestroyDescriptorPool(device, descriptorPool, allocator);
	vkDestroyDescriptorSetLayout(device, setLayout, allocator);
	vkDestroySampler(device, fontSampler, allocator);

	fontImage.cleanUp(device, allocator);
	vertexBuffer.cleanUp(device, allocator);
	indexBuffer.cleanUp(device, allocator);
}
