#include "application.h"
#include "input.h"

#include "systems/resource_system.h"
#include "systems/texture_system.h"
#include "systems/material_system.h"
#include "systems/geometry_system.h"
#include "systems/geometry_utils.h"
#include "systems/shader_system.h"
#include "systems/camera_system.h"
#include "systems/view_system.h"
#include "systems/light_system.h"

#include "resources/transform.h"
#include "resources/geometry.h"

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

		state_.renderer = renderer_frontend::create(backend_type::vulkan, state_.platform);

		resource_system_configuration resource_system_configuration{};
		resource_system_configuration.max_loader_count = 10;
		resource_system_configuration.base_path = "../../../../assets/";

		if (!resource_system::create(resource_system_configuration))
		{
			LOG_FATAL("Failed to create resource system");
		}
		resource_system::init();

		texture_system::create(state_.renderer.get(), { 1024 });
		if (!material_system::create(state_.renderer.get()))
		{
			LOG_FATAL("Failed to create material system");
		}

		if (!geometry_system::create(state_.renderer.get()))
		{
			LOG_FATAL("Failed to create geometry system");
		}

		shader_system_configuration shader_system_configuration{};
		shader_system_configuration.max_global_textures = 31;
		shader_system_configuration.max_instance_textures = 31;
		shader_system_configuration.max_shader_count = 1024;
		shader_system_configuration.max_uniform_count = 128;

		if (!shader_system::create(state_.renderer.get(), shader_system_configuration))
		{
			LOG_FATAL("Failed to create shader system");
		}

		if (!camera_system::create(state_.renderer.get(), { 31 }))
		{
			LOG_FATAL("Failed to create camera system");
		}

		if (!view_system::create(state_.renderer.get()))
		{
			LOG_FATAL("Failed to create view system");
			return;
		}

		if (!light_system::create())
		{
			LOG_FATAL("Failed to create light system");
			return;
		}

		if (!state_.renderer->init())
		{
			LOG_FATAL("Failed to initialise renderer");
		}

		shader_system::init();
		texture_system::init();
		material_system::init();
		geometry_system::init();
		camera_system::init();
		view_system::init();
		light_system::init();

		state_.dir_light_ = std::make_shared<light::directional_light>(
			float4(-0.57735F, -0.57735F, -0.57735F, 1.F),
			float4(0.6F, 0.6F, 0.6F, 1.0F)
		);

		light_system::add_directional_light(state_.dir_light_);
		light_system::add_point_light({ float4(-5.5, -5.5, 0.0, 0.F),
			float4(0.0, 1.0, 0.0, 1.0),
			1.0, // Constant
			0.35, // Linear
			0.44,  // Quadratic
			0.F });

		light_system::add_point_light({
			float4(5.5, -5.5, 0.0, 0.0),
				float4(1.0, 0.0, 0.0, 1.0),
				1.0, // Constant
				0.35, // Linear
				0.44,  // Quadratic
				0.0 });

		light_system::add_point_light({
			float4(5.5, 5.5, 0.0, 0.0),
			float4(0.0, 0.0, 1.0, 1.0),
			1.0, // Constant
			0.35, // Linear
			0.44,  // Quadratic
			0.0 });

		{
			render_view::configuration opaque_world{};
			opaque_world.type = render_view::type::world;
			opaque_world.width = state_.width_;
			opaque_world.height = state_.height_;
			opaque_world.name = "world-opaque";
			opaque_world.passes.push_back({ "Renderpass.Builtin.World" });
			opaque_world.view_source = render_view::view_matrix_source::scene_camera;
			view_system::create_view(opaque_world);
		}
		{
			render_view::configuration ui{
			.name = "ui",
			.width = state_.width_,
			.height = state_.height_,
			.type = render_view::type::ui,
			.view_source = render_view::view_matrix_source::ui_camera,
			};
			ui.passes.push_back({ "Renderpass.Builtin.UI" });
			view_system::create_view(ui);
		}

		if (!state_.game->init())
		{
			LOG_ERROR("FAiled to create game");
		}

		event::register_event(event_code::key_down, nullptr, application::on_event);
		event::register_event(event_code::quit, nullptr, application::on_event);
		event::register_event(event_code::resize, nullptr, application::on_resize);
		event::register_event(event_code::debug01, nullptr, application::on_debug_event);
		event::register_event(event_code::debug02, nullptr, application::on_debug_event);

		auto cube_1 = geometry_system::generate_cube(10, 10, 10, 1, 1, "cube_1", "test_material");
		generate_tangents(cube_1.vertices, cube_1.indices);

		transform transform_1 = transform::create();
		auto mesh_1 = mesh::create(geometry_system::acquire(cube_1), transform_1);
		meshes_.push_back(mesh_1);

		auto cube_2 = geometry_system::generate_cube(5, 5, 5, 1, 1, "cube_2", "test_material");
		generate_tangents(cube_2.vertices, cube_2.indices);

		transform model_2 = transform::create({ 10.F, 0.F, 0.F });
		model_2.set_parent(&mesh_1->model());
		auto mesh_2 = mesh::create(geometry_system::acquire(cube_2), model_2);
		meshes_.push_back(mesh_2);

		auto mesh_resource = resource_system::load("sponza2", resource_type::mesh);

		auto geometry_config = (egkr::vector<geometry::properties>*)mesh_resource->data;
		transform obj = transform::create({ 0, 0, -5 });
		obj.set_scale({ 0.1F, 0.1F, 0.1F });
		obj.set_rotation(glm::quat{ { glm::radians(90.F), 0, 0 } });
		auto mesh = mesh::create();
		mesh->set_model(obj);

		for (const auto& geom : *geometry_config)
		{
			generate_tangents(geom.vertices, geom.indices);
			mesh->add_geometry(geometry_system::acquire(geom));
		}

		meshes_.push_back(mesh);

		resource_system::unload(mesh_resource);

		std::vector<vertex_2d> vertices{4};

		vertices[0] = { {0.F, 512.F}, {0.F, 1.F} };
		vertices[1] = { {512.F, 512.F}, {1.F, 1.F} };
		vertices[2] = { {512.F, 0.F}, {1.F, 0.F} };
		vertices[3] = { {0.F, 0.F}, {0.F, 0.F} };

		std::vector<uint32_t> indices{0, 1, 2, 0, 2, 3};

		geometry::properties ui_properties{};
		ui_properties.name = "test_ui";
		ui_properties.material_name = "ui_material";
		ui_properties.vertex_count = 4;
		ui_properties.vertex_size = sizeof(vertex_2d);
		ui_properties.vertices = malloc(sizeof(vertex_2d) * 4);
		std::copy(vertices.data(), vertices.data() + 4, (vertex_2d*)ui_properties.vertices);
		ui_properties.indices = indices;

		auto ui_geo = geometry::geometry::create(state_.renderer->get_backend().get(), ui_properties);
		ui_meshes_.push_back(mesh::create(ui_geo, {}));

		box = debug::debug_box3d::create(state_.renderer->get_backend().get(), { 0.2, 0.2, 0.2 }, nullptr);
		box->get_transform().set_position(light_system::get_point_lights()[0].position);
		box->load();
		box->set_colour((light_system::get_point_lights()[0].colour));

		debug::configuration grid_configuration{};
		grid_configuration.name = "debug_grid";
		grid_configuration.orientation = debug::orientation::yz;
		grid_configuration.tile_count_dim0 = 100;
		grid_configuration.tile_count_dim1 = 100;
		grid_configuration.tile_scale = 1;

		grid = debug::debug_grid::create(state_.renderer->get_backend().get(), grid_configuration);
		grid->load();
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

				auto& model = application_->meshes_[0]->model();
				glm::quat q({ 0, 0, 0.5F * delta_time });
				model.rotate(q);

				auto& model_2 = application_->meshes_[1]->model();
				model_2.rotate(q);

				render_packet packet{};

				geometry::render_data debug_box{ .geometry = application_->box->get_geometry(), .model = application_->box->get_transform() };
				geometry::render_data debug_grid{ .geometry = application_->grid->get_geometry(), .model = application_->grid->get_transform() };
				render_view::mesh_packet_data world{ .meshes = application_->meshes_, .debug_meshes = { debug_box, debug_grid} };
			
				auto world_view = view_system::get("world-opaque");
				packet.render_views.push_back(view_system::build_packet(world_view.get(), &world));

				render_view::mesh_packet_data ui{ .meshes = application_->ui_meshes_ };
				auto ui_view = view_system::get("ui");
				packet.render_views.push_back(view_system::build_packet(ui_view.get(), &ui));

				state.renderer->draw_frame(packet);
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
		application_->box->destroy();
		application_->grid->unload();
		application_->meshes_.clear();

		application_->ui_meshes_.clear();

		event::unregister_event(event_code::key_down, nullptr, on_event);
		event::unregister_event(event_code::quit, nullptr, on_event);
		event::unregister_event(event_code::resize, nullptr, application::on_resize);

		light_system::shutdown();
		view_system::shutdown();
		shader_system::shutdown();
		texture_system::shutdown();
		material_system::shutdown();
		geometry_system::shutdown();
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
			const std::array<std::string_view, 2> materials{ "Random_Stones", "Seamless" };

			static int choice = 0;
			choice++;
			choice %= materials.size();

			auto material = material_system::acquire(materials[choice]);
			application_->meshes_[0]->get_geometries()[0]->set_material(material);
		}

		if (code == event_code::debug02)
		{
			static bool once = true;
			if (once)
			{
				auto& parent = application_->meshes_[1];
				once = false;
				auto kittyCAD = resource_system::load("output", resource_type::mesh);
				auto kittCAD_config = (egkr::vector<geometry::properties>*)kittyCAD->data;
				transform kittyCAD_transform = transform::create({ 15.F, 0.F, 0.F });
				kittyCAD_transform.set_parent(&parent->model());
				auto kittyCAD_mesh = mesh::create();
				kittyCAD_mesh->set_model(kittyCAD_transform);
				for (const auto& geom : *kittCAD_config)
				{
					generate_tangents(geom.vertices, geom.indices);
					kittyCAD_mesh->add_geometry(geometry_system::acquire(geom));
				}
				application_->meshes_.push_back(kittyCAD_mesh);
			}
		}
		return false;
	}
}