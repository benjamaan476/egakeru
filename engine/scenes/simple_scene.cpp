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
			unload();
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

		std::ranges::for_each(meshes_, [](auto& mesh) { mesh->load(); });

		std::ranges::for_each(debug_boxes_, [](auto& box) { box->load(); });
		std::ranges::for_each(debug_grids_, [](auto& grid) { grid->load(); });
		std::ranges::for_each(debug_frusta_, [](auto& frustum) { frustum->load(); });

		state_ = state::loaded;
	}

	void simple_scene::unload()
	{
		remove_skybox();

		for (const auto& mesh : meshes_)
		{
			remove_mesh(mesh);
		}
		meshes_.clear();

		for (const auto& box : debug_boxes_)
		{
			remove_debug(box);
		}
		debug_boxes_.clear();

		for (const auto& grid : debug_grids_)
		{
			remove_debug(grid);
		}
		debug_grids_.clear();

		for (const auto& frustum : debug_frusta_)
		{
			remove_debug(frustum);
		}
		debug_frusta_.clear();
	}

	void simple_scene::update(const frame_data& /*delta_time*/, const camera::shared_ptr& camera, float aspect)
	{
		if (state_ >= state::loaded)
		{
			auto frustum = egkr::frustum(camera->get_position(), camera->get_forward(), camera->get_right(), camera->get_up(), aspect, camera->get_fov(), camera->get_near_clip(), camera->get_far_clip());
			//if (update_frustum_)
			//{
			//	debug_frustum_ = egkr::debug::debug_frustum::create(camera_frustum_);
			//}

			frame_geometry_.reset();
			for (auto& mesh : meshes_)
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

			for (auto& mesh : meshes_)
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

			for (const auto& debug : debug_boxes_ | std::views::transform([](auto box) { return render_data{ .geometry = box->get_geometry(), .model = box->get_transform() }; }))
			{
				frame_geometry_.debug_geometries.push_back(debug);
			}

			for (const auto& debug : debug_grids_ | std::views::transform([](auto box) { return render_data{ .geometry = box->get_geometry(), .model = box->get_transform() }; }))
			{
				frame_geometry_.debug_geometries.push_back(debug);
			}

			for (const auto& debug : debug_frusta_ | std::views::transform([](auto box) { return render_data{ .geometry = box->get_geometry(), .model = box->get_transform() }; }))
			{
				frame_geometry_.debug_geometries.push_back(debug);
			}

			for (const auto& mesh : meshes_ | std::views::transform([](const auto& mesh) { return mesh->get_debug_data(); }))
			{
				frame_geometry_.debug_geometries.emplace_back(mesh->get_geometry(), mesh->get_transform());
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

	void simple_scene::add_directional_light(std::shared_ptr<light::directional_light>& light)
	{
		if (directional_light_)
		{
			LOG_WARN("Directional light is already set, replacing");
		}
		light_system::add_directional_light(light);

		directional_light_ = light;
	}

	void simple_scene::remove_directional_light()
	{
		light_system::remove_directional_light();
		directional_light_.reset();
	}

	void simple_scene::add_point_light(light::point_light& light)
	{
		light_system::add_point_light(light);

		point_lights_.push_back(light);
	}

	void simple_scene::remove_point_light()
	{
		//light_system::remove_point_light(light);
	}

	void simple_scene::add_mesh(const mesh::shared_ptr& mesh)
	{
		if (state_ >= state::loaded)
		{
			mesh->load();
		}

		meshes_.push_back(mesh);
	}

	void simple_scene::remove_mesh(const mesh::shared_ptr& mesh)
	{
		for(auto& m : meshes_)
		{
			if (m && m->get_id() == mesh->get_id())
			{
				m->unload();
				m.reset();
				break;
			}
		}
	}

	void simple_scene::add_skybox(const skybox::shared_ptr& skybox)
	{
		if (skybox_)
		{
			skybox_->unload();
		}

		if (state_ >= state::loaded)
		{
			skybox_->load();
		}

		skybox_ = skybox;
	}

	void simple_scene::remove_skybox()
	{
		if (state_ >= state::loaded)
		{
			if (skybox_)
			{
				skybox_->unload();
				skybox_.reset();
			}
		}
	}

	void simple_scene::add_debug(const egkr::debug::debug_box3d::shared_ptr& debug_box)
	{
		if (state_ >= state::initialised)
		{
			debug_box->init();
		}

		if (state_ >= state::loaded)
		{
			debug_box->load();
		}

		debug_boxes_.push_back(debug_box);
	}

	void simple_scene::remove_debug(const egkr::debug::debug_box3d::shared_ptr& debug_box)
	{
		for (auto& m : debug_boxes_)
		{
			if (m && m->get_id() == debug_box->get_id())
			{
				m->unload();
				m.reset();
				break;
			}
		}
	}

	void simple_scene::add_debug(const egkr::debug::debug_grid::shared_ptr& debug_grid)
	{

		if (state_ >= state::loaded)
		{
			debug_grid->load();
		}

		debug_grids_.push_back(debug_grid);
	}

	void simple_scene::remove_debug(const egkr::debug::debug_grid::shared_ptr& debug_grid)
	{
		for (auto& m : debug_grids_)
		{
			if (m && m->get_id() == debug_grid->get_id())
			{
				m->unload();
				m.reset();
				break;
			}
		}
	}


	void simple_scene::add_debug(const egkr::debug::debug_frustum::shared_ptr& debug_frustum)
	{
		if (state_ >= state::loaded)
		{
			debug_frustum->load();
		}

		debug_frusta_.push_back(debug_frustum);
	}

	void simple_scene::remove_debug(const egkr::debug::debug_frustum::shared_ptr& debug_frustum)
	{
		for (auto& m : debug_frusta_)
		{
			if (m && m->get_id() == debug_frustum->get_id())
			{
				m->unload();
				m.reset();
				break;
			}
		}
	}

}