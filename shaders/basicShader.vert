#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform pushConstants 
{
    	mat4 view;
	mat4 projection;
} pushConstant;

layout(binding = 0) buffer UniformBufferObject
{
	mat4 model;
	vec4 tint;
	float transparency;
} ubo;

layout(location = 0) in vec3 inPos;

layout(location = 0) out float transparency;
layout(location = 1) out vec4 tint;

void main()
{
	gl_Position = pushConstant.projection * pushConstant.view * ubo.model  * vec4(inPos, 1.0);
	tint = ubo.tint;
	transparency = ubo.transparency;
}

