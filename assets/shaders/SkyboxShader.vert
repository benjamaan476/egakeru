#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in vec4 in_colour;
layout(location = 4) in vec4 in_tangent;

layout(set = 0, binding = 0) uniform global_uniform_object
{
	mat4 projection;
	mat4 view;
} global_ubo;

layout(location = 0) out vec3 tex_coord;

mat4 get_z_correction_matrix()
{
    float s = sin(radians(90.0));
    float c = cos(radians(90.0));
    return mat4(
        1, 0, 0, 0,
        0, c, s, 0,
        0, -s, c, 0,
        0, 0, 0, 1
    );
}

void main() 
{
	tex_coord = in_position;

	gl_Position = global_ubo.projection * global_ubo.view * get_z_correction_matrix() * vec4(in_position, 1.0);
}

