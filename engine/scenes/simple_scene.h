#pragma once
#include "pch.h"
#include "resources/transform.h"
#include "resources/light.h"
#include "resources/mesh.h"
#include "resources/skybox.h"

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

			void update(const frame_data& delta_time);
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
		};
	}
}