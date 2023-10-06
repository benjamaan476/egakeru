#include "renderer_frontend.h"
#include "event.h"

#include "systems/texture_system.h"
#include "systems/material_system.h"
#include "vulkan/renderer_vulkan.h"

namespace egkr
{
	renderer_frontend::unique_ptr renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
	{
		return std::make_unique<renderer_frontend>(type, platform);
	}

	renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
	{
		projection_ = glm::perspective(glm::radians(45.0F), platform->get_framebuffer_size().x / (float)platform->get_framebuffer_size().y, 0.1F, 1000.F);
		float4x4 view{ 1 };
		view = glm::translate(view, { 0.F, 0.F, 30.F });
		view = glm::inverse(view);
		switch (type)
		{
		case backend_type::vulkan:
			backend_ = renderer_vulkan::create(platform);
			break;
		case backend_type::opengl:
		case backend_type::directx:
		default:
			LOG_ERROR("Unsupported renderer backend chosen");
			break;
		}

	}

	bool renderer_frontend::init()
	{
		auto backen_init = backend_->init();


		event::register_event(event_code::debug01, this, &renderer_frontend::on_debug_event);
		return backen_init;
	}

	void renderer_frontend::shutdown()
	{
		test_geometry_.reset();
		backend_->shutdown();
	}

	void renderer_frontend::on_resize(uint32_t width, uint32_t height)
	{
		projection_ = glm::perspective(glm::radians(45.0F), width / (float)height, near_clip_, far_clip_);
		backend_->resize(width, height);
	}

	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame(packet.delta_time))
		{
			static float angle = 0.F;
			angle -= packet.delta_time;
			backend_->update_global_state(projection_, view_, {}, {}, 0);

			float4x4 model{ 1 };
			model = glm::rotate(model, angle, { 0.F, 0.F, 1.F });

			//if (!test_geometry_)
			//{
			//	test_material_ = material_system::acquire("test_material");
			//	if (!test_material_)
			//	{
			//		LOG_WARN("Automatic material load failed. Creating manually");

			//		material_properties properties{};
			//		properties.name = "test_material";
			//		properties.diffuse_colour = float4{ 1.F };
			//		properties.diffuse_map_name = default_texture_name;
			//		test_material_ = material_system::acquire(properties);
			//	}
			//}

			geometry_render_data render_data{};
			render_data.model = model;
			render_data.geometry = test_geometry_;
			render_data.delta_time = packet.delta_time;

			backend_->draw_geometry(render_data);

			backend_->end_frame();
		}
	}

	void renderer_frontend::set_view(const float4x4& view)
	{
		view_ = view;
	}


	bool renderer_frontend::on_debug_event(event_code code, void* /*sender*/, void* listener, const event_context& /*context*/)
	{
		if (code == event_code::debug01)
		{
			auto* frontend = (renderer_frontend*)listener;

			const std::array<std::string_view, 2> textures{ "RandomStones", "Seamless" };

			static int choice = 0;
			choice++;
			choice %= textures.size();
			frontend->test_geometry_->get_material()->set_diffuse_map({ texture_system::acquire(textures[choice]), texture_use::map_diffuse });
		}
		return false;
	}

	void renderer_frontend::create_default_geometry()
	{
		const float scale{ 10.F };
		const egkr::vector<vertex_3d> vertices{ {{-0.5F * scale, -0.5F * scale, 0.F}, {0.F, 0.F} }, { {0.5F * scale, 0.5F * scale, 0.F}, {1.F,1.F} }, { {-0.5F * scale, 0.5F * scale, 0.F}, {0.F,1.F} }, { {0.5F * scale, -0.5F * scale, 0.F}, {1.F,0.F} } };

		const egkr::vector<uint32_t> indices{ 0, 1, 2, 0, 3, 1 };

		geometry_properties properties{};
		properties.indices = indices;
		properties.vertices = vertices;
		properties.name = "test_geometry";
		properties.material_name = "test_material";

		test_geometry_ = geometry::create(backend_->get_context(), properties);
	}
}