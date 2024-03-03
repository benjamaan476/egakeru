#include "sandbox_game.h"

#include "input.h"
#include <systems/view_system.h>
#include <systems/geometry_system.h>
#include <systems/shader_system.h>
#include <systems/light_system.h>
#include <systems/camera_system.h>

#include <systems/audio_system.h>

#include <systems/material_system.h>

#include <renderer/renderer_types.h>

sandbox_game::sandbox_game(const egkr::application_configuration& configuration)
	: game(configuration)
{
	egkr::bitmap_font_configuration bitmap_font_configuration{ .name = "Arial 32", .size = 32, .resource_name = "Arial32" };
	font_system_configuration_ = { .bitmap_font_configurations = { bitmap_font_configuration } , .max_system_font_count = 1,.max_bitmap_font_count = 1 };
}

bool sandbox_game::init()
{
	LOG_INFO("Sandbox game created");

	egkr::event::register_event(egkr::event_code::debug01, this, sandbox_game::on_debug_event);
	egkr::event::register_event(egkr::event_code::debug02, this, sandbox_game::on_debug_event);

	test_text_ = egkr::text::ui_text::create(egkr::text::type::bitmap, "Arial 32", 32, "some test text! \n\t mah~`?^");
	test_text_->set_position({ 50, 250, 0 });

	more_test_text_ = egkr::text::ui_text::create(egkr::text::type::bitmap, "Arial 32", 32, "a");
	more_test_text_->set_position({ 50, 400, 0 });

	skybox_ = egkr::skybox::skybox::create();

	auto skybox_geo = egkr::geometry_system::generate_cube(10, 10, 10, 1, 1, "skybox_cube", "");
	skybox_->set_geometry(egkr::geometry_system::acquire(skybox_geo));

	auto skybox_shader = egkr::shader_system::get_shader("Shader.Builtin.Skybox");
	egkr::vector<egkr::texture_map::texture_map::shared_ptr> maps = { skybox_->get_texture_map() };
	skybox_shader->acquire_instance_resources(maps);

	auto cube_1 = egkr::geometry_system::generate_cube(10, 10, 10, 1, 1, "cube_1", "test_material");

	egkr::transform transform_1 = egkr::transform::create();
	auto mesh_1 = egkr::mesh::create(egkr::geometry_system::acquire(cube_1), transform_1);
	meshes_.push_back(mesh_1);

	auto cube_2 = egkr::geometry_system::generate_cube(5, 5, 5, 1, 1, "cube_2", "test_material");

	egkr::transform model_2 = egkr::transform::create({ 10.F, 0.F, 0.F });
	model_2.set_parent(&mesh_1->model());
	auto mesh_2 = egkr::mesh::create(egkr::geometry_system::acquire(cube_2), model_2);
	meshes_.push_back(mesh_2);

	egkr::light_system::add_point_light({ egkr::float4(-5.5, -5.5, 0.0, 0.F),
	egkr::float4(0.0, 1.0, 0.0, 1.0),
	1.0, // Constant
	0.35, // Linear
	0.44,  // Quadratic
	0.F });

	egkr::light_system::add_point_light({
		egkr::float4(5.5, -5.5, 0.0, 0.0),
			egkr::float4(1.0, 0.0, 0.0, 1.0),
			1.0, // Constant
			0.35, // Linear
			0.44,  // Quadratic
			0.0 });

	egkr::light_system::add_point_light({
		egkr::float4(5.5, 5.5, 0.0, 0.0),
		egkr::float4(0.0, 0.0, 1.0, 1.0),
		1.0, // Constant
		0.35, // Linear
		0.44,  // Quadratic
		0.0 });

	std::vector<vertex_2d> vertices{ 4 };

	vertices[0] = { {0.F, 136.F}, {0.F, 1.F} };
	vertices[1] = { {512.F, 136.F}, {1.F, 1.F} };
	vertices[2] = { {512.F, 0.F}, {1.F, 0.F} };
	vertices[3] = { {0.F, 0.F}, {0.F, 0.F} };

	std::vector<uint32_t> indices{ 0, 1, 2, 0, 2, 3 };

	egkr::geometry::properties ui_properties{};
	ui_properties.name = "test_ui";
	ui_properties.material_name = "ui_material";
	ui_properties.vertex_count = 4;
	ui_properties.vertex_size = sizeof(vertex_2d);
	ui_properties.vertices = malloc(sizeof(vertex_2d) * 4);
	std::copy(vertices.data(), vertices.data() + 4, (vertex_2d*)ui_properties.vertices);
	ui_properties.indices = indices;

	auto ui_geo = egkr::geometry::geometry::create(ui_properties);
	ui_meshes_.push_back(egkr::mesh::create(ui_geo, {}));

	box_ = egkr::debug::debug_box3d::create({ 0.2, 0.2, 0.2 }, nullptr);
	box_->get_transform().set_position(egkr::light_system::get_point_lights()[0].position);
	box_->load();
	box_->set_colour((egkr::light_system::get_point_lights()[0].colour));

	egkr::debug::configuration grid_configuration{};
	grid_configuration.name = "debug_grid";
	grid_configuration.orientation = egkr::debug::orientation::xy;
	grid_configuration.tile_count_dim0 = 100;
	grid_configuration.tile_count_dim1 = 100;
	grid_configuration.tile_scale = 1;
	grid_configuration.use_third_axis = true;

	grid_ = egkr::debug::debug_grid::create(grid_configuration);
	grid_->load();
	camera_ = egkr::camera_system::get_default();
	camera_->set_position({ 0, 0, 0.F });

	dir_light_ = std::make_shared<egkr::light::directional_light>(
		egkr::float4(-0.57735F, -0.57735F, -0.57735F, 1.F),
		egkr::float4(0.6F, 0.6F, 0.6F, 1.0F)
	);

	egkr::light_system::add_directional_light(dir_light_);

	//test_audio = egkr::audio::audio_system::load_chunk("Test.ogg");
	//test_loop_audio = egkr::audio::audio_system::load_chunk("Fire_loop.ogg");
	//test_music = egkr::audio::audio_system::load_stream("Woodland Fantasy.ogg");

	//test_emitter.audio_file = test_loop_audio;
	//test_emitter.looping = true;
	//test_emitter.falloff = 1.F;

	egkr::audio::audio_system::set_master_volume(0.001F);
	egkr::audio::audio_system::set_channel_volume(0, 1.F);
	egkr::audio::audio_system::set_channel_volume(1, 0.75F);
	egkr::audio::audio_system::set_channel_volume(2, 0.5F);
	egkr::audio::audio_system::set_channel_volume(3, 0.25F);
	egkr::audio::audio_system::set_channel_volume(4, 0.F);
	egkr::audio::audio_system::set_channel_volume(5, 0.F);
	egkr::audio::audio_system::set_channel_volume(6, 0.F);
	egkr::audio::audio_system::set_channel_volume(7, 0.4F);

	//egkr::audio::audio_system::play_emitter(6, &test_emitter);
	//egkr::audio::audio_system::play_channel(7, test_music, true);


	return true;
}

void sandbox_game::update(double delta_time)
{
	auto temp_speed{ 50.F };

	if (egkr::input::is_key_down(egkr::key::a))
	{
		camera_->camera_yaw(1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::d))
	{
		camera_->camera_yaw(-1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::e))
	{
		camera_->camera_pitch(-1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::q))
	{
		camera_->camera_pitch(1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::w))
	{
		camera_->move_forward(temp_speed * (float)delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::x))
	{
		camera_->move_up(temp_speed * (float)delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::space))
	{
		camera_->move_down(temp_speed * (float)delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::s))
	{
		camera_->move_back(50.F * (float)delta_time);
	}

	if (egkr::input::was_key_pressed(egkr::key::t))
	{
		egkr::event::fire_event(egkr::event_code::debug01, nullptr, {});
	}

	if (egkr::input::is_key_down(egkr::key::key_0))
	{
		const uint32_t array_size{ 4 };
		egkr::event_context context;
		context.context_ = std::array<uint32_t, array_size>{ 0U };
		egkr::event::fire_event(egkr::event_code::render_mode, nullptr, context);
	}
	if (egkr::input::is_key_down(egkr::key::key_1))
	{
		const uint32_t array_size{ 4 };
		egkr::event_context context;
		context.context_ = std::array<uint32_t, array_size>{ 1U };
		egkr::event::fire_event(egkr::event_code::render_mode, nullptr, context);
	}
	if (egkr::input::is_key_down(egkr::key::key_2))
	{
		const uint32_t array_size{ 4 };
		egkr::event_context context;
		context.context_ = std::array<uint32_t, array_size>{ 2U };
		egkr::event::fire_event(egkr::event_code::render_mode, nullptr, context);
	}
	if (egkr::input::is_key_down(egkr::key::l))
	{
		egkr::event::fire_event(egkr::event_code::debug02, nullptr, {});
	}

	//egkr::audio::audio_system::set_listener_orientation(camera_->get_position(), camera_->get_forward(), camera_->get_up());
	camera_frustum_ = egkr::frustum(camera_->get_position(), camera_->get_forward(), camera_->get_right(), camera_->get_up(), (float)width_ / height_, camera_->get_fov(), camera_->get_near_clip(), camera_->get_far_clip());
	if (update_frustum_)
	{
		debug_frustum_ = egkr::debug::debug_frustum::create(camera_frustum_);
	}

	frame_data.reset();
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

			if (camera_frustum_.intersects_aabb(center, half_extents))
			{
				frame_data.world_geometries.emplace_back(geo, mesh->get_model());
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

		debug_data->set_colour({ 0, 1,0, 0});
		debug_data->set_extents(mesh->extents());

	}

	//egkr::audio::audio_system::update(&frame_data);
}

void sandbox_game::render(egkr::render_packet* render_packet, double delta_time)
{
	auto& model = meshes_[0]->model();
	glm::quat q({ 0, 0, 0.5F * delta_time });
	model.rotate(q);

	auto& model_2 = meshes_[1]->model();
	model_2.rotate(q);

	egkr::geometry::render_data debug_box{ .geometry = box_->get_geometry(), .model = box_->get_transform() };
	egkr::geometry::render_data debug_grid{ .geometry = grid_->get_geometry(), .model = grid_->get_transform() };
	egkr::geometry::render_data debug_frustum_geo{ .geometry = debug_frustum_->get_geometry(), .model = debug_frustum_->get_transform() };
	frame_data.debug_geometries.push_back(debug_box);
	frame_data.debug_geometries.push_back(debug_grid);
	frame_data.debug_geometries.push_back(debug_frustum_geo);

	for (const auto& mesh : meshes_ | std::views::transform([](const auto& mesh) { return mesh->get_debug_data(); }))
	{
		frame_data.debug_geometries.emplace_back(mesh->get_geometry(), mesh->get_transform());
	}

	egkr::render_view::skybox_packet_data skybox{ .skybox = skybox_ };
	auto skybox_view = egkr::view_system::get("skybox");
	render_packet->render_views.push_back(egkr::view_system::build_packet(skybox_view.get(), &skybox));

	auto world_view = egkr::view_system::get("world-opaque");
	render_packet->render_views.push_back(egkr::view_system::build_packet(world_view.get(), &frame_data));

	auto cam = egkr::camera_system::get_default();
	const auto& pos = cam->get_position();
	std::string text = std::format("Camera pos: {} {} {}\n {} meshes drawn", pos.x, pos.y, pos.z, frame_data.world_geometries.size());

	more_test_text_->set_text(text);
	egkr::render_view::ui_packet_data ui{ .mesh_data = {ui_meshes_}, .texts = {test_text_, more_test_text_} };
	auto ui_view = egkr::view_system::get("ui");
	render_packet->render_views.push_back(egkr::view_system::build_packet(ui_view.get(), &ui));

	frame_data.reset();
}

bool sandbox_game::resize(uint32_t width, uint32_t height)
{
	if (width_ != width || height_ != height)
	{
		width_ = width;
		height_ = height;
		return true;
	}

	return false;
}

bool sandbox_game::boot()
{

	{
		egkr::render_view::configuration skybox_world{};
		skybox_world.type = egkr::render_view::type::skybox;
		skybox_world.width = width_;
		skybox_world.height = height_;
		skybox_world.name = "skybox";
		skybox_world.passes.push_back({ "Renderpass.Builtin.Skybox" });
		skybox_world.view_source = egkr::render_view::view_matrix_source::scene_camera;
		render_view_configuration_.push_back(skybox_world);
	}
	{
		egkr::render_view::configuration opaque_world{};
		opaque_world.type = egkr::render_view::type::world;
		opaque_world.width = width_;
		opaque_world.height = height_;
		opaque_world.name = "world-opaque";
		opaque_world.passes.push_back({ "Renderpass.Builtin.World" });
		opaque_world.view_source = egkr::render_view::view_matrix_source::scene_camera;
		render_view_configuration_.push_back(opaque_world);
	}
	{
		egkr::render_view::configuration ui{};
		ui.name = "ui";
		ui.width = width_;
		ui.height = height_;
		ui.type = egkr::render_view::type::ui;
		ui.view_source = egkr::render_view::view_matrix_source::ui_camera;
		ui.passes.push_back({ "Renderpass.Builtin.UI" });
		render_view_configuration_.push_back(ui);
	}

	return true;
}

bool sandbox_game::shutdown()
{
	//egkr::audio::audio_system::shutdown();
	skybox_->destroy();
	box_->destroy();
	grid_->unload();
	std::ranges::for_each(meshes_, [](auto& mesh) { mesh->unload(); });
	meshes_.clear();

	ui_meshes_.clear();
	test_text_.reset();
	more_test_text_.reset();

	debug_frustum_->destroy();

	return true;
}

bool sandbox_game::on_debug_event(egkr::event_code code, void* /*sender*/, void* listener, const egkr::event_context& /*context*/)
{
	auto* game = (sandbox_game*)listener;
	if (code == egkr::event_code::debug01)
	{
		const std::array<std::string_view, 2> materials{ "Random_Stones", "Seamless" };

		static int choice = 0;
		choice++;
		choice %= materials.size();

		auto material = egkr::material_system::acquire(materials[choice]);
		game->meshes_[0]->get_geometries()[0]->set_material(material);
		//game->update_frustum_ = !game->update_frustum_;
	}

	if (code == egkr::event_code::debug02)
	{
		if (!game->models_loaded_)
		{
			LOG_INFO("Loading models");
			game->models_loaded_ = true;

			game->sponza_ = egkr::mesh::load("sponza2");

			egkr::transform obj = egkr::transform::create({ 0, 0, -5 });
			obj.set_scale({ 0.1F, 0.1F, 0.1F });
			obj.set_rotation(glm::quat{ { glm::radians(90.F), 0, 0 } });

			game->sponza_->set_model(obj);
			game->meshes_.push_back(game->sponza_);

			return true;
		}
	}
	return false;
}
