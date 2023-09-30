#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_position;

layout (set = 0, binding = 0) uniform global_uniform_object
{
	mat4 projection;
	mat4 view;
	mat4 reserved0;
	mat4 reserved1;
} global_ubo;

layout(push_constant) uniform push_constant
{
	mat4 model;
} u_push_constant;

void main()
{
	gl_Position = global_ubo.projection * global_ubo.view * u_push_constant.model * vec4(in_position, 1);
}