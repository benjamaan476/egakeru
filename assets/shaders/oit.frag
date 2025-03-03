#version 450

layout(location = 0) out vec4 out_colour;

// Data Transfer Object
layout(location = 1) in struct dto {
	vec4 ambient;
	vec2 tex_coord;
	vec3 normal;
	vec3 view_position;
	vec3 frag_position;
	vec4 colour;
	vec4 tangent;
} out_dto;

void main()
{
	out_colour = vec4(1, 0, 0, 0.5);
}
