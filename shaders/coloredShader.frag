#version 450
#extension GL_ARB_separate_shader_objects : enable


struct ShaderDrawInfo
{
	float transparency;
	vec4 tint;
};

layout(location = 0) in ShaderDrawInfo shaderDrawInfo;
layout(location = 2) in vec4 inFragColor;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 color = inFragColor;
	color.a *= 1 - shaderDrawInfo.transparency;

	FragColor = color;
}