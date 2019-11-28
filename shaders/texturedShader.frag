#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in float transparency;
layout(location = 1) in vec4 tint;
layout(location = 2) in vec2 fragPos;

layout(location = 0) out vec4 FragColor;


void main()
{
	vec4 color = texture(texSampler, fragPos);
	color.a *= 1 - transparency;

	FragColor = color;
}