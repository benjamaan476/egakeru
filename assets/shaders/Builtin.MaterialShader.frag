#version 450

layout (location = 0) in flat int in_mode;
layout (location = 1) in struct dto
{
	vec4 ambient_colour;
	vec3 normal;
	vec3 view_position;
	vec3 frag_position;
	vec2 tex;
} in_dto;

layout (location = 0) out vec4 out_colour;

layout (set = 1, binding = 0) uniform local_uniform_object
{
	vec4 diffuse;
	float shininess;
} object_ubo;

int SAMPLER_DIFFUSE = 0;
int SAMPLER_SPECULAR = 1;

layout (set = 1, binding = 1) uniform sampler2D samplers[2];

struct directional_light
{
	vec3 direction;
	vec4 colour;
};

directional_light light = directional_light(vec3(-0.57735, -0.57735, -0.57735), vec4(0.8, 0.8, 0.8, 1.0));

vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_position)
{
	vec3 half_dir = normalize(view_position - light.direction);

	float diffuse_factor = max(dot(normal, -light.direction), 0.0);
	float specular_factor = pow(max(dot(half_dir, normal), 0.0), object_ubo.shininess);
	vec4 diffuse_sample = texture(samplers[SAMPLER_DIFFUSE], in_dto.tex);

	vec4 ambient = vec4(vec3(in_dto.ambient_colour * object_ubo.diffuse).xyz, diffuse_sample.a);
	vec4 diffuse = vec4(vec3(light.colour * diffuse_factor).xyz, diffuse_sample.a);
	vec4 specular = vec4(vec3(light.colour * specular_factor).xyz, diffuse_sample.a);

	diffuse *= diffuse_sample;
	ambient *= diffuse_sample;
	specular *= vec4(texture(samplers[SAMPLER_SPECULAR], in_dto.tex).rgb, diffuse_sample.a);

	return ambient + diffuse + specular;
}

void main()
{


	vec3 view_position = normalize(in_dto.view_position - in_dto.frag_position);
	out_colour = calculate_directional_light(light, in_dto.normal, view_position);
}