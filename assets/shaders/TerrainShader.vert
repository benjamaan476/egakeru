#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in vec4 in_colour;
layout(location = 4) in vec4 in_tangent;
layout(location = 5) in vec4 in_mat_weights;

struct directional_light {
    vec4 direction;
    vec4 colour;
};

const int MAX_POINT_LIGHTS = 10;
const int MAX_MATERIALS = 4;

struct point_light {
    vec4 position;
    vec4 colour;
    // Usually 1, make sure denominator never gets smaller than 1
    float constant;
    // Reduces light intensity linearly
    float linear;
    // Makes the light fall off slower at longer distances.
    float quadratic;

    float pad;
};
struct material_info
{
    vec4 diffuse_colour;
    float shininess;
    vec3 pad;
};

layout(set = 0, binding = 0) uniform global_uniform_object {
    mat4 projection;
    mat4 view;
    mat4 model;
    vec4 ambient_colour;
    vec3 view_position;
    int mode;
    material_info materials[MAX_MATERIALS];
    directional_light dir_light;
    point_light point_lights[MAX_POINT_LIGHTS];
    int num_point_lights;
    int num_materials;
} global_ubo;

// layout(push_constant) uniform push_constants {
//
//     // Only guaranteed a total of 128 bytes.
//     mat4 model; // 64 bytes
// } u_push_constants;

layout(location = 0) out int out_mode;

// Data Transfer Object
layout(location = 1) out struct dto {
    vec4 ambient;
    vec2 tex_coord;
    vec3 normal;
    vec3 view_position;
    vec3 frag_position;
    vec4 colour;
    vec4 tangent;
    vec4 material_weights;
} out_dto;

void main() {
    out_dto.tex_coord = in_texcoord;
    out_dto.colour = in_colour;
    // Fragment position in world space.
    out_dto.frag_position = vec3(global_ubo.model * vec4(in_position, 1.0));
    // Copy the normal over.
    mat3 m3_model = mat3(global_ubo.model);
    out_dto.normal = normalize(m3_model * in_normal);
    out_dto.tangent = vec4(normalize(m3_model * in_tangent.xyz), in_tangent.w);
    out_dto.ambient = global_ubo.ambient_colour;
    out_dto.view_position = global_ubo.view_position;
    out_dto.material_weights = in_mat_weights;
    gl_Position = global_ubo.projection * global_ubo.view * global_ubo.model * vec4(in_position, 1.0);

    out_mode = global_ubo.mode;
}
