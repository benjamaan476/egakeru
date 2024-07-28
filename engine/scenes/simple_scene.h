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

#include "ray.h"

namespace egkr
{
	struct render_packet;
	namespace scene
	{
		enum class state
		{
			uninitialised,
			unloading,
			unloaded,
			initialised,
			loading,
			loaded,
		};

		struct skybox_scene_configuration
		{
			std::string name;
			std::string resource_name;
		};

		struct directional_light_scene_configuration
		{
			std::string name;
			egkr::float4 colour;
			egkr::float4 direction;
		};

		struct mesh_scene_configuration
		{
			std::string name;
			std::string resource_name;
			transform transform;
			std::optional<std::string> parent_name;
		};

		struct point_light_scene_configuration
		{
			std::string name;
			egkr::float4 colour;
			egkr::float4 position;
			float constant;
			float linear;
			float quadratic;
		};

		struct configuration
		{
			std::string name;
			std::string description;
			skybox_scene_configuration skybox{};
			directional_light_scene_configuration directional_light;
			std::vector<mesh_scene_configuration> meshes;
			std::vector<point_light_scene_configuration> point_lights;
		};

		struct pending_mesh
		{
			mesh::shared_ptr mesh;
			egkr::vector<geometry::properties> geometries;
			std::string resource_name;
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
			void add_directional_light(const std::string& name, std::shared_ptr<light::directional_light>& light);
			void remove_directional_light();

			void add_point_light(const std::string& name, light::point_light& light);
			void remove_point_light(const std::string& name);

			void add_mesh(const std::string& name, const mesh::shared_ptr& mesh);
			void remove_mesh(const std::string& name);

			void add_skybox(const std::string& name, const skybox::shared_ptr& skybox);
			void remove_skybox(const std::string& name);

			void add_debug(const std::string& name, const egkr::debug::debug_box3d::shared_ptr& debug_box);
			void remove_debug(const std::string& name);
			void add_debug(const std::string& name, const egkr::debug::debug_grid::shared_ptr& debug_grid);
			void add_debug(const std::string& name, const egkr::debug::debug_frustum::shared_ptr& debug_frustum);

			[[nodiscard]] bool is_loaded() const { return state_ >= state::loaded; }

			ray::result raycast(const ray& ray);
		private:
			void actual_unload();

		private:
			//configuration configuration_{};
			uint32_t id_{};
			state state_{state::uninitialised};
			transform transform_;
			//bool is_enabled_{};

			std::string skybox_name_;
			skybox::shared_ptr skybox_;
			std::string directional_light_name_;
			std::shared_ptr<light::directional_light> directional_light_;
			std::unordered_map<std::string, light::point_light> point_lights_;

			std::queue<pending_mesh> pending_meshes_;
			std::unordered_map<std::string, mesh::shared_ptr> meshes_;

			std::unordered_map<std::string, egkr::debug::debug_box3d::shared_ptr> debug_boxes_;
			std::unordered_map<std::string, egkr::debug::debug_grid::shared_ptr> debug_grids_;
			std::unordered_map<std::string, egkr::debug::debug_frustum::shared_ptr> debug_frusta_;

			frame_geometry_data frame_geometry_{};
		};
	}
}
