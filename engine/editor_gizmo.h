#pragma once

#include "pch.h"

#include "renderer/vertex_types.h"
#include "resources/geometry.h"
#include "resources/transform.h"

namespace egkr::editor
{
	class gizmo
	{
	public:
		enum class mode
		{
			none,
			move,
			rotate,
			scale
		};

		struct gizmo_data
		{
			egkr::vector<colour_vertex_3d> vertices;
			egkr::vector<int32_t> indices;
			geometry::shared_ptr geo;
		};

		[[nodiscard]] static gizmo create();

		gizmo();
		~gizmo();

		void init();
		void destroy();

		void load();
		void unload();
		void update();

		void set_mode(mode mode);
	private:
		void setup_none();
		void setup_move();
		void setup_rotate();
		void setup_scale();
	private:
		transform transform{};
		mode gizmo_mode{ mode::none };
		std::unordered_map<mode, gizmo_data> mode_data;
	};
}
