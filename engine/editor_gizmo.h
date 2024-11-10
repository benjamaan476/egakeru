#pragma once

#include "pch.h"

#include "renderer/vertex_types.h"
#include "interfaces/renderable.h"
#include "resources/transform.h"
#include "ray.h"

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

		enum class interaction_type : uint8_t
		{
			none,
			mouse_hover,
			mouse_down,
			mouse_drag,
			mouse_up,
			cancel
		};

		struct gizmo_data : public renderable
		{
			egkr::vector<colour_vertex_3d> vertices;
			egkr::vector<uint32_t> indices;

			egkr::vector<extent3d> extents;
			uint8_t current_axis_index{ 255 };
			std::optional<plane> interaction_plane;
			float3 interaction_start;
			float3 last_interaction_point;
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
		ray::hit_result raycast(const ray& ray);

		void begin_interaction(interaction_type type, const ray& ray);
		void end_interaction(interaction_type type, const ray& ray);
		void handle_interaction(interaction_type type, const ray& ray);

		void set_selected(const std::weak_ptr<transformable>& transform);
		std::optional<float4x4> get_selected_model() const;

		static void set_scale(float scale_factor);

	private:
		void setup_none();
		void setup_move();
		void setup_rotate();
		void setup_scale();
	private:
		mode gizmo_mode{ mode::none };
		std::unordered_map<mode, gizmo_data> mode_data;
		interaction_type type_{ interaction_type::none };
		uint32_t unique_id{ invalid_32_id };
		std::weak_ptr<transformable> selected_transform;
	};
}
