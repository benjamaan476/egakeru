#include "viewport.h"

namespace egkr
{
	viewport viewport::create(const float4& rect, projection_type type, float fov, float near_clip, float far_clip)
	{
		return viewport(rect, type, fov, near_clip, far_clip);
	}

	viewport::viewport(const float4& rect, projection_type projection_type, float viewport_fov, float near, float far)
		: viewport_rect(rect), type(projection_type), fov(viewport_fov), near_clip(near), far_clip(far)
	{

		update_projection();
	}

	void viewport::resize(const float4& rect)
	{
		viewport_rect = rect;
		update_projection();
	}

	void viewport::update_projection()
	{
		switch (type)
		{
		case projection_type::perspective:
		{
			projection = glm::perspective(fov, viewport_rect.z / viewport_rect.w, near_clip, far_clip);
			break;
		case projection_type::orthographic:
			projection = glm::ortho(0.f, viewport_rect.z, viewport_rect.w, 0.f, near_clip, far_clip);
			break;
		default:
			LOG_ERROR("Invalid projection type!");
			break;
		}
		}
	}
}
