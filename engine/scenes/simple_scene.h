#pragma once
#include "pch.h"
#include "resources/transform.h"
#include "resources/light.h"
#include "resources/mesh.h"
#include "resources/skybox.h"
#include "renderer/camera.h"

#include "debug/debug_box3d.h"
#include "debug/debug_grid.h"
#include "debug/debug_frustum.h"

#include <queue>

namespace egkr
{
	struct render_packet;
	namespace scene
	{
		enum class state
		{
			uninitialised,
			unloaded,
			initialised,
			loading,
			loaded,
		};

		struct configuration
		{

		};

		struct pending_mesh
		{
			mesh::shared_ptr mesh{};
			egkr::vector<geometry::properties> geometries{};
			std::string resource_name{};
		};

		class simple_scene
		{
		public:
			using unique_ptr = std::unique_ptr<simple_scene>;
			static unique_ptr create(const configuration& configuration);

			explicit simple_scene(const configuration& configuration);

			void init();
			void destroy();
			void load();
			void unload();

			void update(const frame_data& delta_time, const camera::shared_ptr& camera, float aspect);
			void populate_render_packet(render_packet* packet);

			//Game owns these, scene just references them
			void add_directional_light(std::shared_ptr<light::directional_light>& light);
			void remove_directional_light();

			void add_point_light(light::point_light& light);
			void remove_point_light();

			void add_mesh(const mesh::shared_ptr& mesh);
			void remove_mesh(const mesh::shared_ptr& mesh);

			void add_skybox(const skybox::shared_ptr& skybox);
			void remove_skybox();

			void add_debug(const egkr::debug::debug_box3d::shared_ptr& debug_box);
			void remove_debug(const egkr::debug::debug_box3d::shared_ptr& debug_box);
			void add_debug(const egkr::debug::debug_grid::shared_ptr& debug_grid);
			void remove_debug(const egkr::debug::debug_grid::shared_ptr& debug_grid);
			void add_debug(const egkr::debug::debug_frustum::shared_ptr& debug_frustum);
			void remove_debug(const egkr::debug::debug_frustum::shared_ptr& debug_frustum);

		private:
			//configuration configuration_{};
			uint32_t id_{};
			state state_{state::uninitialised};
			transform transform_{};
			//bool is_enabled_{};

			skybox::shared_ptr skybox_{};
			std::shared_ptr<light::directional_light> directional_light_{};
			std::vector<light::point_light> point_lights_{};

			std::queue<pending_mesh> pending_meshes_{};
			std::vector<mesh::shared_ptr> meshes_{};

			std::vector< egkr::debug::debug_box3d::shared_ptr> debug_boxes_{};
			std::vector< egkr::debug::debug_grid::shared_ptr> debug_grids_{};
			std::vector< egkr::debug::debug_frustum::shared_ptr> debug_frusta_{};

			frame_geometry_data frame_geometry_{};
		};
	}
}