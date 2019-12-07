#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec4 tint;
layout(location = 1) in float transparency;
layout(location = 2) in vec4 fragColor;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 color =  mix(fragColor, vec4(tint.rgb, 1.0), tint.a) ;
	color.a *= 1 - transparency;

	FragColor = color;
}