#version 450
# extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in flat int in_mode;
layout (location = 1) in struct dto
{
	vec2 tex;
} in_dto;

layout (location = 0) out vec4 out_colour;

layout (set = 1, binding = 0) uniform local_uniform_object
{
	vec4 diffuse;
} object_ubo;

layout (set = 1, binding = 1) uniform sampler2D diffuse_sampler;

void main()
{
	out_colour = texture(diffuse_sampler, in_dto.tex) * object_ubo.diffuse;
}