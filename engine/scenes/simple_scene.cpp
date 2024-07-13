#include "simple_scene.h"
#include "identifier.h"
#include <systems/view_system.h>
#include <systems/light_system.h>
#include <renderer/renderer_types.h>

namespace egkr::scene
{
	simple_scene::unique_ptr simple_scene::create(const configuration& configuration)
	{
		return std::make_unique<simple_scene>(configuration);
	}

	simple_scene::simple_scene(const configuration& /*configuration*/)
//		: configuration_{ configuration }
	{
		id_ = identifier::acquire_unique_id(this);
	}

	void simple_scene::init()
	{
		state_ = state::initialised;
	}

	void simple_scene::destroy()
	{
		if (state_ == state::loaded)
		{
			actual_unload();
		}

		meshes_.clear();
		point_lights_.clear();
		frame_geometry_.reset();
		state_ = state::uninitialised;
	}

	void simple_scene::load()
	{
		if (skybox_)
		{
			skybox_->load();
		}

		std::ranges::for_each(meshes_ | std::views::values, [](auto& mesh) { mesh->load(); });

		std::ranges::for_each(debug_boxes_ | std::views::values, [](auto& box) { box->load(); });
		std::ranges::for_each(debug_grids_ | std::views::values, [](auto& grid) { grid->load(); });
		std::ranges::for_each(debug_frusta_ | std::views::values, [](auto& frustum) { frustum->load(); });

		state_ = state::loaded;
	}

	void simple_scene::unload()
	{
		state_ = state::unloading;
	}

	void simple_scene::update(const frame_data& /*delta_time*/, const camera::shared_ptr& camera, float aspect)
	{
		if (state_ == state::unloading)
		{
			actual_unload();
		}

		if (state_ >= state::loaded)
		{
			auto frustum = egkr::frustum(camera->get_position(), camera->get_forward(), camera->get_right(), camera->get_up(), aspect, camera->get_fov(), camera->get_near_clip(), camera->get_far_clip());
			//if (update_frustum_)
			//{
			//	debug_frustum_ = egkr::debug::debug_frustum::create(camera_frustum_);
			//}

			frame_geometry_.reset();
			for (auto& mesh : meshes_ | std::views::values)
			{
				if (mesh->get_generation() == invalid_32_id)
				{
					continue;
				}

				auto model_world = mesh->model().get_world();
				for (const auto& geo : mesh->get_geometries())
				{
					auto extents_max = model_world * egkr::float4{ geo->get_properties().extents.max, 1.F };
					egkr::float3 t{ extents_max };
					egkr::float3 center = model_world * egkr::float4{ geo->get_properties().center, 1.F };

					const egkr::float3 half_extents{ glm::abs(t - center) };

					if (frustum.intersects_aabb(center, half_extents))
					{
						frame_geometry_.world_geometries.emplace_back(geo, mesh->get_model());
					}
				}
			}

			for (auto& mesh : meshes_ | std::views::values)
			{
				if (mesh->get_generation() == invalid_32_id)
				{
					continue;
				}
				auto& debug_data = mesh->get_debug_data();
				if (!debug_data)
				{
					debug_data = egkr::debug::debug_box3d::create({ 0.2, 0.2, 0.2 }, &mesh->model());
					debug_data->load();
				}

				debug_data->set_colour({ 0, 1,0, 0 });
				debug_data->set_extents(mesh->extents());

			}

			for (const auto& debug : debug_boxes_ | std::views::values | std::views::transform([](auto box) { return render_data{ .geometry = box->get_geometry(), .model = box->get_transform() }; }))
			{
				frame_geometry_.debug_geometries.push_back(debug);
			}

			for (const auto& debug : debug_grids_ | std::views::values | std::views::transform([](auto box) { return render_data{ .geometry = box->get_geometry(), .model = box->get_transform() }; }))
			{
				frame_geometry_.debug_geometries.push_back(debug);
			}

			for (const auto& debug : debug_frusta_ | std::views::values | std::views::transform([](auto box) { return render_data{ .geometry = box->get_geometry(), .model = box->get_transform() }; }))
			{
				frame_geometry_.debug_geometries.push_back(debug);
			}

			for (const auto& mesh : meshes_ | std::views::values | std::views::transform([](const auto& mesh) { return mesh->get_debug_data(); }))
			{
				if (mesh)
				{
					frame_geometry_.debug_geometries.emplace_back(mesh->get_geometry(), mesh->get_transform());
				}
			}

		}
	}

	void simple_scene::populate_render_packet(render_packet* packet)
	{
		skybox_packet_data skybox{ .skybox = skybox_ };
		packet->render_views[render_view::type::skybox] = view_system::build_packet(view_system::get("skybox").get(), &skybox);

		auto world_view = view_system::get("world-opaque");
		packet->render_views[render_view::type::world] = view_system::build_packet(world_view.get(), &frame_geometry_);

	}

	void simple_scene::add_directional_light(const std::string& name, std::shared_ptr<light::directional_light>& light)
	{
		if (directional_light_)
		{
			LOG_WARN("Directional light is already set, replacing");
		}
		directional_light_name_ = name;
		light_system::add_directional_light(light);

		directional_light_ = light;
	}

	void simple_scene::remove_directional_light()
	{
		light_system::remove_directional_light();
		directional_light_.reset();
		directional_light_name_ = "";
	}

	void simple_scene::add_point_light(const std::string& name, light::point_light& light)
	{
		light_system::add_point_light(light);

		point_lights_.emplace(name, light);
	}

	void simple_scene::remove_point_light(const std::string& name)
	{
		//LOG_WARN("Cannot currently remove a point light");
				//light_system::remove_point_light(light);
		if (point_lights_.contains(name))
		{
			point_lights_.erase(name);
			return;
		}

		LOG_WARN("Point light, {}, not added to scene. Cannot remove", name);
	}

	void simple_scene::add_mesh(const std::string& name, const mesh::shared_ptr& mesh)
	{
		if (state_ >= state::loaded)
		{
			mesh->load();
		}

		meshes_.emplace(name, mesh);
	}

	void simple_scene::remove_mesh(const std::string& name)
	{
		if (meshes_.contains(name))
		{
			auto& m = meshes_[name];
			m->unload();
			m.reset();
		}
	}

	void simple_scene::add_skybox(const std::string& name, const skybox::shared_ptr& skybox)
	{
		if (skybox_)
		{
			skybox_->unload();
		}

		if (state_ >= state::loaded)
		{
			skybox->load();
		}

		skybox_ = skybox;
		skybox_name_ = name;
	}

	void simple_scene::remove_skybox(const std::string& /*name*/)
	{
		if (state_ >= state::loaded || state_ == state::unloading)
		{
			if (skybox_)
			{
				skybox_->unload();
				skybox_.reset();
			}
		}
		skybox_name_ = "";
	}

	void simple_scene::add_debug(const std::string& name, const egkr::debug::debug_box3d::shared_ptr& debug_box)
	{
		if (state_ >= state::initialised)
		{
			debug_box->init();
		}

		if (state_ >= state::loaded)
		{
			debug_box->load();
		}

		debug_boxes_.emplace(name, debug_box);
	}

	void simple_scene::remove_debug(const std::string& name)
	{
		if (debug_boxes_.contains(name))
		{
			auto& m = debug_boxes_[name];
			m->unload();
			m.reset();
			//debug_boxes_.erase(name);
			LOG_INFO("Removed debug box {}", name);
			return;
		}

		if (debug_grids_.contains(name))
		{
			auto& m = debug_grids_[name];
			m->unload();
			m.reset();
			//debug_grids_.erase(name);
			LOG_INFO("Removed debug grid {}", name);
			return;
		}

		if (debug_frusta_.contains(name))
		{
			auto& m = debug_frusta_[name];
			m->unload();
			m.reset();
			//debug_frusta_.erase(name);
			LOG_INFO("Removed debug frustum {}", name);
			return;
		}

		LOG_WARN("Tried to remove debug item {} that was not in scene", name);
	}

	void simple_scene::add_debug(const std::string& name, const egkr::debug::debug_grid::shared_ptr& debug_grid)
	{

		if (state_ >= state::loaded)
		{
			debug_grid->load();
		}

		debug_grids_.emplace(name, debug_grid);
	}

	void simple_scene::add_debug(const std::string& name, const egkr::debug::debug_frustum::shared_ptr& debug_frustum)
	{
		if (state_ >= state::loaded)
		{
			debug_frustum->load();
		}

		debug_frusta_.emplace(name, debug_frustum);
	}

	void simple_scene::actual_unload()
	{
		remove_skybox(skybox_name_);

		for (const auto& mesh : meshes_ | std::views::keys)
		{
			remove_mesh(mesh);
		}
		meshes_.clear();

		for (const auto& box : debug_boxes_ | std::views::keys)
		{
			remove_debug(box);
		}
		debug_boxes_.clear();

		for (const auto& grid : debug_grids_ | std::views::keys)
		{
			remove_debug(grid);
		}
		debug_grids_.clear();

		for (const auto& frustum : debug_frusta_ | std::views::keys)
		{
			remove_debug(frustum);
		}
		debug_frusta_.clear();

		state_ = state::unloaded;
	}
}
