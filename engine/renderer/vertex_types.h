#pragma once

struct vertex_2d
{
	egkr::float2 position{};
	egkr::float2 tex{};
};

struct vertex_3d
{
	//Must match the vertex input attribute description
	egkr::float3 position{};
	egkr::float3 normal{};
	egkr::float2 tex{};
	egkr::float4 colour{};
	egkr::float4 tangent{};
};
