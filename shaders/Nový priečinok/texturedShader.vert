#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 0) out vec2 fragPos;

void main()
{
	gl_Position = ubo.projection * ubo.view * ubo.model  * vec4(inPos, 1.0);
	fragPos;= inTexCoord;
}

