#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec2 inTex;

layout(location = 1) out vec2 fragTex;

void main()
{
	//uint vertexIndex = gl_VertexIndex % 6;

	//uint cornerIndex = vertexIndex > 2 ? (vertexIndex - 1) % 4 : vertexIndex;

	vec3 corners[4] = vec3[4](vec3(-1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, -1.0, 1.0), vec3(-1.0, -1.0, 1.0));
	vec2 uv[4] = vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0));
	vec3 pos = corners[gl_VertexIndex];
	pos *= 0.5;
	gl_Position = vec4(pos, 1);
	fragTex = uv[gl_VertexIndex];
}