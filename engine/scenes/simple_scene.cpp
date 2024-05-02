#include "simple_scene.h"
#include "identifier.h"
#include <systems/view_system.h>
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
		state_ = state::uninitialised;
	}

	void simple_scene::load()
	{
		if (skybox_)
		{
			skybox_->load();
		}

		std::ranges::for_each(meshes_, [](auto& mesh) { mesh->load(); });

		state_ = state::loaded;
	}

	void simple_scene::unload()
	{
		remove_skybox();

		for (const auto& mesh : meshes_)
		{
			remove_mesh(mesh);
		}
	}

	void simple_scene::update(const frame_data& /*delta_time*/)
	{
	}

	void simple_scene::populate_render_packet(render_packet* packet)
	{
		egkr::skybox_packet_data skybox{ .skybox = skybox_ };
		packet->render_views[egkr::render_view::type::skybox] = view_system::build_packet(view_system::get("skybox").get(), &skybox);

	}

	void simple_scene::add_directional_light(std::shared_ptr<light::directional_light>& light)
	{
		if (directional_light_)
		{
			LOG_WARN("Directional light is already set, replacing");
		}

		directional_light_ = light;
	}

	void simple_scene::remove_directional_light()
	{
		directional_light_.reset();
	}

	void simple_scene::add_point_light(light::point_light& /*light*/)
	{
	}

	void simple_scene::remove_point_light()
	{
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
}