#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 FragColor;

layout(location = 0) in float transparency;
layout(location = 1) in vec4 tint;

void main()
{
	vec4 color= mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(tint.rgb, 1.0), tint.a);
	color.a *= 1 - transparency;
	FragColor = color;
}