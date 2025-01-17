#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_colour;

layout(set = 0, binding = 0) uniform global_uniform
{
	mat4 proj;
	mat4 view;
} global_ubo;

layout(push_constant) uniform push_constants 
{
	mat4 model;
} local_ubo;

layout(location = 1) out struct dto
{
	vec4 colour;
} out_dto;

void main()
{
	out_dto.colour = in_colour;
	gl_Position = global_ubo.proj * global_ubo.view * local_ubo.model * in_position;
}