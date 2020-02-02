#version 450
#extension GL_ARB_separate_shader_objects : enable

struct ShaderDrawInfo
{
	float transparency;
	vec4 tint;
};

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in ShaderDrawInfo shaderDrawInfo;
layout(location = 2) in vec2 fragPos;

layout(location = 0) out vec4 FragColor;


void main()
{
	//vec4 color = mix(texture(texSampler, fragPos), vec4(shaderDrawInfo.tint.rgb, 1.0), shaderDrawInfo.tint.a);
	vec4 color = texture(texSampler, fragPos);
	color.a *= 1 - shaderDrawInfo.transparency;

	FragColor = color;
}