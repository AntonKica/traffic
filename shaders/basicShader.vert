#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform pushConstants 
{
    	mat4 view;
	mat4 projection;
} pushConstant;

layout(binding = 0) buffer UniformBufferObject
{
	mat4 model;
} ubo;

layout(location = 0) in vec3 inPos;

void main()
{
	gl_Position = pushConstant.projection * pushConstant.view * ubo.model  * vec4(inPos, 1.0);
}

