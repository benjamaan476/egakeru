#pragma once
#include "pch.h"

namespace egkr
{
	enum class projection_type
	{
		perspective,
		orthographic
	};

	struct viewport
	{
		viewport() = default;
		float4 viewport_rect{0};
		projection_type type;
		float4x4 projection{ 1.f };
		float fov{};
		float near_clip{};
		float far_clip{};

		static viewport create(const float4& rect, projection_type type, float fov, float near_clip, float far_clip);

		void resize(const float4& rect);

	private:
		viewport(const float4& rect, projection_type type, float fov, float near_clip, float far_clip);
		void update_projection();
	};
}
