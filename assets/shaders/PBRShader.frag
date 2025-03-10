#version 450

layout(location = 0) out vec4 out_colour;

struct directional_light {
    vec4 direction;
    vec4 colour;
};

const int MAX_POINT_LIGHTS = 10;

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

struct pbr_properties {
    vec4 diffuse;
    vec3 pad;
    float shininess;
};

layout(set = 1, binding = 0) uniform local_uniform_object {
    directional_light dir_light;
    point_light point_lights[MAX_POINT_LIGHTS];
    int num_point_lights;
    pbr_properties props;
} object_ubo;

// Samplers, diffuse, spec
const int SAMP_ALBEDO = 0;
const int SAMP_NORMAL = 1;
const int SAMP_METALLIC = 2;
const int SAMP_ROUGHNESS = 3;
const int SAMP_AO = 4;
const int SAMP_SHADOW = 5;
const int SAMP_IBL = 6;
const int SAMP_COUNT = 7;

layout(set = 1, binding = 1) uniform sampler2D samplers[SAMP_COUNT];
layout(set = 1, binding = 1) uniform samplerCube samplersCube[SAMP_COUNT];

const float PI = 3.14159265359;

layout(location = 0) flat in int in_mode;
layout(location = 1) in struct dto {
    vec4 ambient;
    vec2 tex_coord;
    vec3 normal;
    vec3 view_position;
    vec3 frag_position;
    vec4 colour;
    vec4 tangent;
    vec4 light_space_frag_position;
} in_dto;

mat3 TBN;

float geometry_ggx(float normal_dot_direction, float roughness)
{
    roughness += 1;
    float k = (roughness * roughness) / 8;
    return normal_dot_direction / (normal_dot_direction * (1 - k) + k);
}

vec3 calculate_point_light_radiance(point_light light, vec3 view_direction, vec3 frag_position)
{
    float distance = length(light.position.xyz - frag_position);
    float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    return light.colour.rgb * attenuation;
}

vec3 calculate_directional_light_radiance(directional_light light)
{
    return light.colour.rgb;
}

vec3 calculate_reflectance(vec3 albedo, vec3 normal, vec3 view_direction, vec3 light_direction, float metallic, float roughness, vec3 base_reflectivity, vec3 radiance)
{
    vec3 halfway = normalize(view_direction + light_direction);
    float rough_sq = roughness * roughness;
    float rough_sq_sq = rough_sq * rough_sq;

    float normal_dot_halfway = max(dot(normal, halfway), 0);
    float normal_dot_halfway_sq = normal_dot_halfway * normal_dot_halfway;

    float denom = normal_dot_halfway_sq * (rough_sq_sq - 1) + 1;
    denom = PI * denom * denom;

    float normal_distribution = (rough_sq_sq / denom);

    float normal_dot_view_dir = max(dot(normal, view_direction), 0);
    float normal_dot_light_dir = max(dot(normal, light_direction), 0);

    float ggx_0 = geometry_ggx(normal_dot_view_dir, roughness);
    float ggx_1 = geometry_ggx(normal_dot_light_dir, roughness);

    float geo = ggx_0 * ggx_1;

    float cos_theta = max(dot(halfway, view_direction), 0);
    vec3 fresnel = base_reflectivity + (1 - base_reflectivity) * pow(clamp(1 - cos_theta, 0, 1), 5);

    vec3 numerator = normal_distribution * geo * fresnel;
    float denominator = 4 * max(dot(normal, view_direction), 0) + 0.0001;

    vec3 specular = numerator / denominator;
    vec3 refraction_diffuse = vec3(1) - fresnel;
    refraction_diffuse *= 1 - metallic;

    return (refraction_diffuse * albedo / PI + specular) * radiance * normal_distribution;
}

float calculate_shadow(vec4 light_space_frag_position) {
    vec3 projected = light_space_frag_position.xyz / light_space_frag_position.w;
    //NDC
    projected = projected * 0.5 + 0.5;

    float closest_depth = texture(samplers[SAMP_SHADOW], projected.xy).r;
    float current_depth = projected.z;

    float shadow = current_depth > closest_depth ? 1 : 0;
    return shadow;
}

void main() {
    vec3 normal = in_dto.normal;
    vec3 tangent = in_dto.tangent.xyz;
    tangent = (tangent - dot(tangent, normal) * normal);

    vec3 bitangent = cross(in_dto.normal, in_dto.tangent.xyz);
    TBN = mat3(tangent, bitangent, normal);

    vec3 local_normal = 2 * texture(samplers[SAMP_NORMAL], in_dto.tex_coord).rgb - 1;
    normal = normalize(TBN * local_normal);

    vec4 albedo_samp = texture(samplers[SAMP_ALBEDO], in_dto.tex_coord);

    //Gamma correction
    vec3 albedo = pow(albedo_samp.rgb, vec3(2.2));

    float metallic = texture(samplers[SAMP_METALLIC], in_dto.tex_coord).r;
    float roughness = texture(samplers[SAMP_ROUGHNESS], in_dto.tex_coord).r;
    float ao = texture(samplers[SAMP_AO], in_dto.tex_coord).r;

    vec3 base_reflectivity = vec3(0.04);
    base_reflectivity = mix(base_reflectivity, albedo, metallic);

    if (in_mode == 0 || in_mode == 1)
    {
        vec3 view_direction = in_dto.view_position - in_dto.frag_position;
        albedo += vec3(1) * in_mode;

        albedo = clamp(albedo, vec3(0), vec3(1));

        vec3 total_reflectance = vec3(0);

        {
            directional_light light = object_ubo.dir_light;
            vec3 light_dir = normalize(-light.direction.xyz);
            vec3 radiance = calculate_directional_light_radiance(light);

            total_reflectance += calculate_reflectance(albedo, normal, view_direction, light_dir, metallic, roughness, base_reflectivity, radiance);
        }

        for (int i = 0; i < object_ubo.num_point_lights; i++)
        {
            point_light light = object_ubo.point_lights[i];

            vec3 light_dir = normalize(light.position.xyz - in_dto.frag_position.xyz);

            vec3 radiance = calculate_point_light_radiance(light, in_dto.view_position, in_dto.frag_position);

            total_reflectance += calculate_reflectance(albedo, normal, view_direction, light_dir, metallic, roughness, base_reflectivity, radiance);
        }

        vec3 irradiance = texture(samplersCube[SAMP_IBL], normal).rgb;
        float shadow = calculate_shadow(in_dto.light_space_frag_position);

        vec3 ambient = irradiance * albedo * ao;
        // vec3 colour = ambient + total_reflectance;
        vec3 colour = vec3(shadow, shadow, shadow);
        colour = colour / (colour + vec3(1));
        colour = pow(colour, vec3(1 / 2.2));

        out_colour = vec4(colour, albedo_samp.a);
    }
    else if (in_mode == 2)
    {
        out_colour = vec4(abs(normal), 1);
    }
}
