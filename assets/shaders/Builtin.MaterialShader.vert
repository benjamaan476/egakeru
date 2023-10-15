#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex;

layout(location = 0) out flat int mode;
layout(location = 1) out struct dto
{
	vec4 ambient_colour;
	vec3 normal;
	vec3 view_position;
	vec3 frag_position;
	vec2 tex;
} out_dto;

layout (set = 0, binding = 0) uniform global_uniform_object
{
	mat4 projection;
	mat4 view;
	vec4 ambient_colour;
	vec3 view_position;
} global_ubo;

layout(push_constant) uniform push_constant
{
	mat4 model;
} u_push_constant;

void main()
{
	out_dto.ambient_colour = global_ubo.ambient_colour;
	out_dto.normal = mat3(u_push_constant.model) * in_normal;
	out_dto.view_position = global_ubo.view_position;
	out_dto.frag_position = vec3(u_push_constant.model * vec4(in_position, 1.0));
	out_dto.tex = in_tex;
	gl_Position = global_ubo.projection * global_ubo.view * u_push_constant.model * vec4(in_position, 1);
}