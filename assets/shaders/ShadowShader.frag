#version 460

layout(set = 1, binding = 1) uniform sampler2D colour;

layout(location = 1) in struct dto {
    vec2 tex;
} in_dto;

layout(location = 0) out vec4 out_colour;

void main() {
    out_colour = vec4(1);
}
