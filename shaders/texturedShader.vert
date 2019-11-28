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
	float transparency;
	vec4 tint;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out float transparency;
layout(location = 1) out vec4 tint;
layout(location = 2) out vec2 fragPos;

void main()
{
	gl_Position = pushConstant.projection * pushConstant.view * ubo.model  * vec4(inPos, 1.0);
	transparency = ubo.transparency;
	tint = ubo.tint;
	fragPos = inTexCoord;
}

