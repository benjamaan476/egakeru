#pragma once

#include "pch.h"

#include "renderer/vertex_types.h"
#include "renderable.h"
#include "transformable.h"

namespace egkr::editor
{
	class gizmo : public transformable
	{
	public:
		enum class mode
		{
			none,
			move,
			rotate,
			scale
		};

		struct gizmo_data : public renderable
		{
			egkr::vector<colour_vertex_3d> vertices;
			egkr::vector<uint32_t> indices;

			friend gizmo;
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

		void draw() const { mode_data.at(gizmo_mode).draw(); }
	private:
		void setup_none();
		void setup_move();
		void setup_rotate();
		void setup_scale();
	private:
		mode gizmo_mode{ mode::none };
		std::unordered_map<mode, gizmo_data> mode_data;
	};
}
