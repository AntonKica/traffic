#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 FragColor;


void main()
{
	FragColor = texture(texSampler, fragTexCoord);
	//FragColor = vec4(fragTexCoord, 0.0, 1.0);
}