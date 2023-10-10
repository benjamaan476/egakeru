#include "application.h"
#include "input.h"

#include "systems/resource_system.h"
#include "systems/texture_system.h"
#include "systems/material_system.h"
#include "systems/geometry_system.h"

using namespace std::chrono_literals;

namespace egkr
{

	static application::unique_ptr application_;
	bool application::create(game::unique_ptr game)
	{
		if (!application_)
		{
			application_ = std::make_unique<application>(std::move(game));
			application_->state_.game->set_application(application_.get());
			return true;
		}

		LOG_WARN("Application already initialised");
		return false;
	}

	application::application(game::unique_ptr game)
	{
		state_.width_ = game->get_application_configuration().width;
		state_.height_ = game->get_application_configuration().height;
		state_.name = game->get_application_configuration().name;
		state_.game = std::move(game);

		//Init subsystems

		egkr::log::init();
		egkr::input::init();
		//
		const uint32_t start_x = 100;
		const uint32_t start_y = 100;

		const platform_configuration platform_config = { .start_x = start_x, .start_y = start_y, .width_ = state_.width_, .height_ = state_.height_, .name = state_.name };



		state_.platform = egkr::platform::create(egkr::platform_type::windows);
		if (state_.platform == nullptr)
		{
			LOG_FATAL("Failed to create platform");
			return;
		}

		auto success = state_.platform->startup(platform_config);
		if (!success)
		{
			LOG_FATAL("Failed to start platform");
			return;
		}

		resource_system_configuration resource_system_configuration{};
		resource_system_configuration.max_loader_count = 6;
		resource_system_configuration.base_path = "../assets/";

		if (!resource_system::create(resource_system_configuration))
		{
			LOG_FATAL("Failed to create resource system");
		}

		state_.renderer = renderer_frontend::create(backend_type::vulkan, state_.platform);
		if (!state_.renderer->init())
		{
			LOG_FATAL("Failed to initialise renderer");
		}



		texture_system::create(state_.renderer->get_backend_context(), { 1024 });
		if(!material_system::create(state_.renderer->get_backend_context()))
		{
			LOG_FATAL("Failed to create material system");
		}

		if (!geometry_system::create(state_.renderer->get_backend_context()))
		{
			LOG_FATAL("Failed to create geometry system");
		}

		if (!state_.game->init())
		{
			LOG_ERROR("FAiled to create game");
		}

		event::register_event(event_code::key_down, nullptr, application::on_event);
		event::register_event(event_code::quit, nullptr, application::on_event);
		event::register_event(event_code::resize, nullptr, application::on_resize);
		event::register_event(event_code::debug01, nullptr, application::on_debug_event);

		if (!test_geometry_)
		{
			test_geometry_ = geometry_system::get_default();
			if (!test_geometry_)
			{
				LOG_WARN("Automatic material load failed. Creating manually");

			}
		}

		std::vector<vertex_2d> vertices{4};

		vertices[0] = { {0.F, 512.F}, {0.F, 1.F} };
		vertices[1] = { {512.F, 512.F}, {1.F, 1.F} };
		vertices[2] = { {512.F, 0.F}, {1.F, 0.F} };
		vertices[3] = { {0.F, 0.F}, {0.F, 0.F} };

		std::vector<uint32_t> indices{0, 1, 2, 0, 2, 3};

		geometry_properties ui_properties{};
		ui_properties.name = "test_ui";
		ui_properties.material_name = "ui_material";
		ui_properties.vertex_count = 4;
		ui_properties.vertex_size = sizeof(vertex_2d);
		ui_properties.vertices = malloc(sizeof(vertex_2d) * 4);
		std::copy(vertices.data(), vertices.data() + 4, (vertex_2d*)ui_properties.vertices);
		ui_properties.indices = indices;

		test_ui_geometry_ = geometry::create(state_.renderer->get_backend_context(), ui_properties);

		state_.is_running = true;
		is_initialised_ = true;
	}

	void application::run()
	{
		auto& state = application_->state_;
		while (state.is_running)
		{
			auto time = state.platform->get_time();
			std::chrono::duration<double, std::ratio<1, 1>> delta = time - application_->last_time_;
			auto delta_time = delta.count();

			auto frame_time = time;
			state.platform->pump();

			if (!state.is_suspended)
			{
				state.game->update(delta_time);

				state.game->render(delta_time);

				static float angle = 0.F;
				//angle -= delta_time;

				float4x4 model{ 1 };
				model = glm::rotate(model, angle, { 0.F, 0.F, 1.F });

				render_packet render_data{};
				render_data.delta_time = delta_time;
				render_data.world_geometry_data = { { application_->test_geometry_ , model, delta_time} };
				render_data.ui_geometry_data = { { application_->test_ui_geometry_ , model, delta_time} };

				state.renderer->draw_frame(render_data);
			}
			auto frame_duration = state.platform->get_time() - frame_time;
			if (application_->limit_framerate_ && frame_duration < application_->frame_time_)
			{
				auto time_remaining = application_->frame_time_ - frame_duration;
				state.platform->sleep(time_remaining);
			}

			application_->last_time_ = time;
		}
	}

	void application::shutdown()
	{
		application_->test_geometry_.reset();
		application_->test_ui_geometry_.reset();

		event::unregister_event(event_code::key_down, nullptr, on_event);
		event::unregister_event(event_code::quit, nullptr, on_event);
		event::unregister_event(event_code::resize, nullptr, application::on_resize);

		geometry_system::shutdown();
		material_system::shutdown();
		texture_system::shutdown();
		application_->state_.renderer->shutdown();
		application_->state_.platform->shutdown();
	}

	bool application::on_event(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::quit)
		{
			application_->state_.is_running = false;
		}

		if (code == event_code::key_down)
		{
			const size_t array_size{ 8 };
			auto key = (egkr::key)std::get<std::array<int16_t, array_size>>(context)[0];

			switch (key)
			{
			case egkr::key::esc:
				application_->state_.is_running = false;
				break;
			default:
				break;
			}
		}

		return false;
	}

	bool application::on_resize(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::resize)
		{
			const auto& context_array = std::get<std::array<int32_t, 4>>(context);
			const auto& width = context_array[0];
			const auto& height = context_array[1];

			if (application_->state_.width_ != (uint32_t)width || application_->state_.height_ != (uint32_t)height)
			{
				application_->state_.width_ = (uint32_t)width;
				application_->state_.height_ = (uint32_t)height;

				if (width == 0 && height == 0)
				{
					application_->state_.is_suspended = true;
					return false;
				}

				if (application_->state_.is_suspended)
				{
					application_->state_.is_suspended = false;
				}


				application_->state_.game->resize(width, height);
				application_->state_.renderer->on_resize(width, height);

			}
		}
		return false;
	}

	bool application::on_debug_event(event_code code, void* /*sender*/, void* /*listener*/, const event_context& /*context*/)
	{
		if (code == event_code::debug01)
		{
			const std::array<std::string_view, 2> textures{ "RandomStones", "Seamless" };

			static int choice = 0;
			choice++;
			choice %= textures.size();
			application_->test_geometry_->get_material()->set_diffuse_map({ texture_system::acquire(textures[choice]), texture_use::map_diffuse });
		}
		return false;
	}
}