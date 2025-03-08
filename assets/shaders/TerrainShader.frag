#version 450

layout(location = 0) out vec4 out_colour;

struct directional_light {
    vec4 direction;
    vec4 colour;
};

const int MAX_POINT_LIGHTS = 10;
const int MAX_MATERIALS = 8;

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

// layout(set = 1, binding = 0) uniform local_uniform_object {
//     vec4 diffuse_colour;
//     directional_light dir_light;
//     point_light point_lights[MAX_POINT_LIGHTS];
//     int num_point_lights;
//     float shininess;
// } object_ubo;

// // Samplers, diffuse, spec
const int SAMP_DIFFUSE = 0;
const int SAMP_SPECULAR = 1;
const int SAMP_NORMAL = 2;
layout(set = 1, binding = 1) uniform sampler2D samplers[3 * MAX_MATERIALS];

layout(location = 0) flat in int in_mode;
// Data Transfer Object
layout(location = 1) in struct dto {
    vec4 ambient;
    vec2 tex_coord;
    vec3 normal;
    vec3 view_position;
    vec3 frag_position;
    vec4 colour;
    vec4 tangent;
    vec4 material_weights;
} in_dto;

mat3 TBN;

vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_direction, vec4 diff_samp, vec4 spec_samp, material_info mat_info);
vec4 calculate_point_light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction, vec4 diff_samp, vec4 spec_sam, material_info mat_infop);

void main() {
    vec3 normal = in_dto.normal;
    vec3 tangent = in_dto.tangent.xyz;
    tangent = (tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(in_dto.normal, in_dto.tangent.xyz) * in_dto.tangent.w;
    TBN = mat3(tangent, bitangent, normal);

    vec3 norm = vec3(0);
    vec4 diff = vec4(0);
    vec4 spec = vec4(0);

    material_info mat;
    mat.diffuse_colour = vec4(0);
    mat.shininess = 1;

    for (int m = 0; m < global_ubo.num_materials; m++)
    {
        // Update the normal to use a sample from the normal map.
        vec3 localNormal = 2.0 * texture(samplers[3 * m + SAMP_NORMAL], in_dto.tex_coord).rgb - 1.0;
        norm = mix(norm, normalize(TBN * localNormal), in_dto.material_weights[m]);

        diff = mix(diff, texture(samplers[3 * m + SAMP_DIFFUSE], in_dto.tex_coord), in_dto.material_weights[m]);
        spec = mix(spec, texture(samplers[3 * m + SAMP_SPECULAR], in_dto.tex_coord), in_dto.material_weights[m]);
        mat.diffuse_colour = mix(mat.diffuse_colour, global_ubo.materials[m].diffuse_colour, in_dto.material_weights[m]);
        mat.shininess = mix(mat.shininess, global_ubo.materials[m].shininess, in_dto.material_weights[m]);
    }

    if (in_mode == 0 || in_mode == 1) {
        vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);

        out_colour = calculate_directional_light(global_ubo.dir_light, norm, view_direction, diff, spec, mat);

        for (int i = 0; i < global_ubo.num_point_lights; i++)
        {
            out_colour += calculate_point_light(global_ubo.point_lights[i], norm, in_dto.frag_position, view_direction, diff, spec, mat);
        }
    } else if (in_mode == 2) {
        out_colour = vec4(abs(normal), 1.0);
    }
}

vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_direction, vec4 diff_samp, vec4 spec_samp, material_info mat_info) {
    float diffuse_factor = max(dot(normal, -light.direction.xyz), 0.0);

    vec3 half_direction = normalize(view_direction - light.direction.xyz);
    float specular_factor = pow(max(dot(half_direction, normal), 0.0), mat_info.shininess);

    vec4 ambient = vec4(vec3(in_dto.ambient * mat_info.diffuse_colour), diff_samp.a);
    vec4 diffuse = vec4(vec3(light.colour * diffuse_factor), diff_samp.a);
    vec4 specular = vec4(vec3(light.colour * specular_factor), diff_samp.a);

    if (in_mode == 0) {
        diffuse *= diff_samp;
        ambient *= diff_samp;
        specular *= spec_samp;
    }

    return (ambient + diffuse + specular);
}

vec4 calculate_point_light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction, vec4 diff_samp, vec4 spec_samp, material_info mat_info) {
    vec3 light_direction = normalize(light.position.xyz - frag_position);
    float diff = max(dot(normal, light_direction), 0.0);

    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), mat_info.shininess);

    // Calculate attenuation, or light falloff over distance.
    float distance = length(light.position.xyz - frag_position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec4 ambient = in_dto.ambient;
    vec4 diffuse = light.colour * diff;
    vec4 specular = light.colour * spec;

    if (in_mode == 0) {
        diffuse *= diff_samp;
        ambient *= diff_samp;
        specular *= vec4(texture(samplers[SAMP_SPECULAR], in_dto.tex_coord).rgb, diffuse.a);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
