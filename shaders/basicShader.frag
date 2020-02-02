#version 450
#extension GL_ARB_separate_shader_objects : enable

struct ShaderDrawInfo
{
	float transparency;
	vec4 tint;
};

layout(location = 0) in ShaderDrawInfo shaderDrawInfo;
layout(location = 0) out vec4 FragColor;

void main()
{
	//vec4 color= mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(shaderDrawInfo.tint.rgb, 1.0), shaderDrawInfo.tint.a);
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	color.a *= 1 - shaderDrawInfo.transparency;
	FragColor = color;
}