#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec2 fragTex;

void main()
{
	gl_Position = inPosition;
	fragColour = inColour;
	fragTex = inTex;
}