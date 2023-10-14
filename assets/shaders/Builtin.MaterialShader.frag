#version 450

layout (location = 0) in flat int in_mode;
layout (location = 1) in struct dto
{
	vec4 ambient_colour;
	vec3 normal;
	vec2 tex;
} in_dto;

layout (location = 0) out vec4 out_colour;

layout (set = 1, binding = 0) uniform local_uniform_object
{
	vec4 diffuse;
} object_ubo;

layout (set = 1, binding = 1) uniform sampler2D diffuse_sampler;

struct directional_light
{
	vec3 direction;
	vec4 colour;
};

directional_light light = directional_light(vec3(-0.57735, -0.57735, -0.57735), vec4(0.8, 0.8, 0.8, 1.0));

vec4 calculate_directional_light(directional_light light, vec3 normal)
{
	float diffuse_factor = max(dot(normal, -light.direction), 0.0);
	vec4 diffuse_sample = texture(diffuse_sampler, in_dto.tex);

	vec4 ambient = vec4(vec3(in_dto.ambient_colour * object_ubo.diffuse).xyz, diffuse_sample.a);
	vec4 diffuse = vec4(vec3(light.colour * diffuse_factor).xyz, diffuse_sample.a);

	diffuse *= diffuse_sample;
	ambient *= diffuse_sample;

	return ambient + diffuse;
}

void main()
{
	out_colour = calculate_directional_light(light, in_dto.normal);
}