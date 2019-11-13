#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;

layout(location = 0) in vec2 inPos;

layout(location = 0) out vec3 fragColor;

void main()
{
	gl_Position = ubo.projection * ubo.view * ubo.model  * vec4(inPos, 0.0, 1.0);
	fragColor = vec3(inPos, 0.0);
}

