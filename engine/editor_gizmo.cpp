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
	}

	void gizmo::load()
	{
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
		constexpr float axis_length{ 5.f };
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
		constexpr float axis_length{ 5.f };

		for (uint32_t i{ 0u }; i < 6; i += 2)
		{
			float4 position{ 0.f, 0.f, 0.f, 1.f };
			float4 colour{0.f, 0.f, 0.f, 1.f};
			vertices[i] = { .position = position, .colour = colour };
			position[i / 2] = axis_length;
			colour[i / 2] = 1.f;
			vertices[i + 1] = { .position = position, .colour = colour };
		}
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
	}

	void gizmo::setup_scale()
	{

	}

	void gizmo::setup_rotate()
	{

	}
}
