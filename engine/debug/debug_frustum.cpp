#include "debug_frustum.h"

namespace egkr::debug
{
	debug_frustum::shared_ptr debug_frustum::create(const renderer_backend* backend, const egkr::frustum& frustum)
	{
		return std::make_shared<debug_frustum>(backend, frustum);
	}

	debug_frustum::debug_frustum(const renderer_backend* backend, const egkr::frustum& frustum)
		: backend_{ backend }
	{
		vertices_.resize(2 * 12);

		recalculate_lines(frustum);

		geometry::properties properties{};
		properties.name = "debug_frustum";
		properties.vertex_count = vertices_.size();
		properties.vertex_size = sizeof(colour_vertex_3d);
		properties.vertices = vertices_.data();

		geometry_ = geometry::geometry::create(backend_, properties);

		geometry_->increment_generation();
	}

	void debug_frustum::update(const frustum& frustum)
	{
		recalculate_lines(frustum);

		geometry_->update_vertices(0, vertices_.size(), vertices_.data());
	}

	void debug_frustum::destroy()
	{
		if (geometry_)
		{
			geometry_->destroy();
			geometry_.reset();
		}
	}

	void debug_frustum::recalculate_lines(const frustum& frustum)
	{
		const float half_v = frustum.far * std::tanf(0.5f * frustum.fov);
		const float half_h = half_v * frustum.aspect;

		const auto half_right = half_h * frustum.right;
		const auto half_up = half_v * frustum.up;

		const auto& forward = frustum.forward * frustum.far;
		const auto& pos = frustum.position;
		const auto& near = frustum.near;
		const auto& far = frustum.far;

		const float3 up_right = glm::normalize(forward + half_right + half_up);
		const float3 up_left = glm::normalize(forward - half_right + half_up);
		const float3 down_right = glm::normalize(forward + half_right - half_up);
		const float3 down_left = glm::normalize(forward - half_right - half_up);

		vertices_[0].position = {pos + near * up_right , 1.F};
		vertices_[1].position = {pos + far * up_right, 1.F};
		vertices_[2].position = {pos + near * down_right , 1.F};
		vertices_[3].position = {pos + far * down_right, 1.F};
		vertices_[4].position = {pos + near * up_left , 1.F};
		vertices_[5].position = {pos + far * up_left, 1.F};
		vertices_[6].position = {pos + near * down_left , 1.F};
		vertices_[7].position = {pos + far * down_left, 1.F};

		vertices_[8].position = { pos + near * up_right, 1.F };
		vertices_[9].position = { pos + near * up_left, 1.F };
		vertices_[10].position = { pos + near * up_left, 1.F };
		vertices_[11].position = { pos + near * down_left, 1.F };
		vertices_[12].position = { pos + near * down_left, 1.F };
		vertices_[13].position = { pos + near * down_right, 1.F };
		vertices_[14].position = { pos + near * down_right, 1.F };
		vertices_[15].position = { pos + near * up_right, 1.F };

		vertices_[16].position = { pos + far * up_right, 1.F };
		vertices_[17].position = { pos + far * up_left, 1.F };
		vertices_[18].position = { pos + far * up_left, 1.F };
		vertices_[19].position = { pos + far * down_left, 1.F };
		vertices_[20].position = { pos + far * down_left, 1.F };
		vertices_[21].position = { pos + far * down_right, 1.F };
		vertices_[22].position = { pos + far * down_right, 1.F };
		vertices_[23].position = { pos + far * up_right, 1.F };

		std::ranges::for_each(vertices_, [](auto& vertex) { vertex.colour = { 1, 1, 0, 1 }; });
	}


}