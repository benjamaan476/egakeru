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

	bool operator==(const vertex_3d& rhs) const
	{
		return position == rhs.position && normal == rhs.normal && tex == rhs.tex && colour == rhs.colour && tangent == rhs.tangent;
	}
};

struct colour_vertex_3d
{
	egkr::float4 position{0.F, 0.F, 0.F, 1.F};
	egkr::float4 colour{0.F, 0.F, 0.F, 1.F};
};