#version 450

layout(location = 0) in vec3 tex_coord;
layout(location = 0) out vec4 out_colour;

const int SAMPLER_DIFFUSE = 0;

layout(set = 1, binding = 0) uniform samplerCube cube_sampler[1];

void main()
{
	out_colour = texture(cube_sampler[SAMPLER_DIFFUSE], tex_coord);
}
