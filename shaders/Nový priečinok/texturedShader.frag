#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec2 fragPos;

layout(location = 0) out vec4 FragColor;

Sampler2D

void main()
{
	FragColor = texture(texSampler, fragPos);
}