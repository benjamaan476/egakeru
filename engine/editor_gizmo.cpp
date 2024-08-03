#include "editor_gizmo.h"

namespace egkr::editor
{
	gizmo gizmo::create()
	{
		return gizmo();
	}

	gizmo::gizmo()
	{
	}

	gizmo::~gizmo()
	{
		unload();
		destroy();
	}

	void gizmo::init()
	{
		setup_none();
		setup_move();
		setup_rotate();
		setup_scale();
	}

	void gizmo::destroy()
	{
		for (const auto& data : mode_data | std::views::values)
		{
			data.geometry_->destroy();
		}
		mode_data.clear();
	}

	void gizmo::load()
	{
		for (auto& data : mode_data | std::views::values)
		{
			geometry::properties properties
			{
				.vertex_size = sizeof(colour_vertex_3d),
				.vertex_count = (uint32_t)data.vertices.size(),
				.vertices = (void*)data.vertices.data(),

				.indices = data.indices
			};
			data.geometry_ = geometry::create(properties);
			data.geometry_->upload();
		}
	}

	void gizmo::unload()
	{
	}

	void gizmo::update()
	{
	}

	void gizmo::set_mode(mode mode)
	{
		gizmo_mode = mode;
	}

	void gizmo::setup_none()
	{
		constexpr float4 gray{ 0.6f, 0.6f, 0.6f, 1.f };
		constexpr float axis_length{ 1.f };
		egkr::vector<colour_vertex_3d> vertices(6);
		vertices[0] = { .position = {0.F, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[1] = { .position = {axis_length, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[2] = { .position = {0.F, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[3] = { .position = {0.F, axis_length, 0.F, 1.F}, .colour = gray };
		vertices[4] = { .position = {0.F, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[5] = { .position = {0.F, 0.F, axis_length, 1.F}, .colour = gray };

		mode_data[gizmo::mode::none] = { .vertices = vertices };
	}

	void gizmo::setup_move()
	{
		egkr::vector<colour_vertex_3d> vertices(18);
		float axis_length{ 1.f };

		for (uint32_t i{ 0u }; i < 6; i += 2)
		{
			float4 position{ 0.f, 0.f, 0.f, 1.f };
			float4 colour{0.f, 0.f, 0.f, 1.f};
			position[i / 2] = 0.2f;
			colour[i / 2] = 1.f;
			vertices[i] = { .position = position, .colour = colour };
			position[i / 2] = axis_length;
			colour[i / 2] = 1.f;
			vertices[i + 1] = { .position = position, .colour = colour };
		}
		axis_length = 0.8f;
		float4 colour{ 1.f, 0.f, 0.f, 1.f };
		vertices[6] = { .position = {axis_length / 2, 0.f, 0.f, 1.f}, .colour = colour };
		vertices[7] = { .position = {axis_length / 2, axis_length / 2, 0.f, 1.f}, .colour = colour };

		vertices[8] = { .position = {axis_length / 2, 0.f, 0.f, 1.f}, .colour = colour };
		vertices[9] = { .position = {axis_length / 2, 0.f, axis_length / 2, 1.f}, .colour = colour };
		
		colour = { 0.f, 1.f, 0.f, 1.f };
		vertices[10] = { .position = {0.f, axis_length / 2, 0.f, 1.f}, .colour = colour  };
		vertices[11] = { .position = {axis_length / 2, axis_length / 2, 0.f, 1.f}, .colour = colour };

		vertices[12] = { .position = {0.f, axis_length / 2, 0.f, 1.f}, .colour = colour };
		vertices[13] = { .position = {0.f, axis_length / 2, axis_length / 2, 1.f}, .colour = colour };
		
		colour = { 0.f, 0.f, 1.f, 1.f };
		vertices[14] = { .position = {0.f, 0.f, axis_length / 2, 1.f}, .colour = colour  };
		vertices[15] = { .position = {axis_length / 2, 0.f, axis_length / 2, 1.f}, .colour = colour };

		vertices[16] = { .position = {0.f, 0.f, axis_length / 2, 1.f}, .colour = colour };
		vertices[17] = { .position = {0.f, axis_length / 2, axis_length / 2, 1.f}, .colour = colour };

		mode_data[gizmo::mode::move] = { .vertices = vertices };
	}

	void gizmo::setup_scale()
	{
		egkr::vector<colour_vertex_3d> vertices(12);
		float axis_length{ 1.f };

		for (uint32_t i{ 0u }; i < 6; i += 2)
		{
			float4 position{ 0.f, 0.f, 0.f, 1.f };
			float4 colour{ 0.f, 0.f, 0.f, 1.f };
			position[i / 2] = 0.2f;
			colour[i / 2] = 1.f;
			vertices[i] = { .position = position, .colour = colour };
			position[i / 2] = axis_length;
			colour[i / 2] = 1.f;
			vertices[i + 1] = { .position = position, .colour = colour };
		}
		axis_length = 0.8f;
		const float4 red{ 1.f, 0.f, 0.f, 1.f };
		const float4 green{ 0.f, 1.f, 0.f, 1.f };
		const float4 blue{ 0.f, 0.f, 1.f, 1.f };

		vertices[6] = { .position = {axis_length / 2, 0.f, 0.f, 1.f}, .colour = red };
		vertices[7] = { .position = {0, axis_length / 2, 0.f, 1.f}, .colour = green };

		vertices[8] = { .position = {0.f, axis_length / 2, 0.f, 1.f}, .colour = green };
		vertices[9] = { .position = {0.f, 0.f, axis_length / 2, 1.f}, .colour = blue };

		vertices[10] = { .position = {0.f, 0.f, axis_length / 2, 1.f}, .colour = blue };
		vertices[11] = { .position = {axis_length / 2, 0.f, 0.f, 1.f}, .colour = red };

		mode_data[gizmo::mode::scale] = { .vertices = vertices };
	}

	void gizmo::setup_rotate()
	{
		constexpr int8_t segment_count{ 32 };
		constexpr int16_t vertex_count{ 3 * 2 * segment_count };
		egkr::vector<colour_vertex_3d> vertices;
		vertices.reserve(vertex_count);
		const float axis_length{ 1.f };
		constexpr const float two_pi{ 2.f * std::numbers::pi_v<float> };

		float4 colour{ 1.f, 0.f, 0.f, 1.f };
		for (uint8_t i{ 0u }; i < segment_count; ++i)
		{
			float4 position{ 0.f, std::sinf(axis_length * i * two_pi / segment_count), std::cosf(axis_length * i * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);

			position = { 0.f, std::sinf(axis_length * (i + 1) * two_pi / segment_count), std::cosf(axis_length * (i + 1) * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);
		}

		colour = { 0.f, 1.f, 0.f, 1.f };
		for (uint8_t i{ 0u }; i < segment_count; ++i)
		{
			float4 position{ std::sinf(axis_length * i * two_pi / segment_count), 0.f, std::cosf(axis_length * i * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);

			position = { std::sinf(axis_length * (i + 1) * two_pi / segment_count), 0.f, std::cosf(axis_length * (i + 1) * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);
		}

		colour = { 0.f, 0.f, 1.f, 1.f };
		for (uint8_t i{ 0u }; i < segment_count; ++i)
		{
			float4 position{ std::cosf(axis_length * i * two_pi / segment_count), std::sinf(axis_length * i * two_pi / segment_count), 0.f, 1.f };
			vertices.emplace_back(position, colour);

			position = { std::cosf(axis_length * (i + 1) * two_pi / segment_count), std::sinf(axis_length * (i + 1) * two_pi / segment_count), 0.f, 1.f };
			vertices.emplace_back(position, colour);
		}

		mode_data[gizmo::mode::rotate] = { .vertices = vertices };
	}
}
