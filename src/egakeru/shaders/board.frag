#version 450

layout(location = 0) out vec4 outColour;
layout(location = 1) in vec2 fragTex;

layout(binding = 0) uniform UniformBufferObject
{
	vec4 primaryColour;
	vec4 secondaryColour;
	ivec2 size;
} boardProperties;


void main()
{
	vec2 pos = floor((fragTex * boardProperties.size));
	float mask = mod(pos.x + mod(pos.y, 2.0), 2.0);
	outColour = mix(boardProperties.primaryColour, boardProperties.secondaryColour, mask);
}