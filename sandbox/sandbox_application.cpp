#include "sandbox_application.h"
#include "sandbox_keybinds.h"

#include <systems/view_system.h>
#include <systems/geometry_system.h>
#include <systems/light_system.h>
#include <systems/camera_system.h>
#include <systems/audio_system.h>
#include <systems/material_system.h>

#include <systems/resource_system.h>

#include <renderer/renderer_types.h>
#include <debug/debug_console.h>


sandbox_application::sandbox_application(const egkr::engine_configuration& configuration)
	: application(configuration)
{
	egkr::bitmap_font_configuration bitmap_font_configuration{ .name = "Arial 32", .size = 32, .resource_name = "Arial32" };
	egkr::system_font_configuration system_font_configuration{ .name = "NotoSansCJK", .default_size = 24, .resource_name = "NotoSansCJK" };
	font_system_configuration_ = { .system_font_configurations = { system_font_configuration }, .bitmap_font_configurations = { bitmap_font_configuration } , .max_system_font_count = 1,.max_bitmap_font_count = 1 };
}

bool sandbox_application::init()
{
	LOG_INFO("Sandbox application created");

	egkr::event::register_event(egkr::event::code::debug01, this, sandbox_application::on_debug_event);
	egkr::event::register_event(egkr::event::code::debug02, this, sandbox_application::on_debug_event);
	egkr::event::register_event(egkr::event::code::debug03, this, sandbox_application::on_debug_event);

	test_text_ = egkr::text::ui_text::create(egkr::text::type::bitmap, "Arial 32", 32, "some test text! \n\t mah~`?^");
	test_text_->set_position({ 50, 250, 0 });

	more_test_text_ = egkr::text::ui_text::create(egkr::text::type::bitmap, "Arial 32", 32, "a");
	more_test_text_->set_position({ 50, 400, 0 });

	main_scene_ = egkr::scene::simple_scene::create({});

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

	egkr::mesh::configuration ui_mesh_properties{};
	ui_mesh_properties.geometry_configurations.push_back(ui_properties);
	ui_mesh_ = egkr::mesh::create(ui_mesh_properties);
	ui_mesh_->load();
	ui_meshes_.push_back(ui_mesh_);

	camera_ = egkr::camera_system::get_default();
	camera_->set_position({ 0, 0, 0.F });

//TODO add to scene
	test_audio = egkr::audio::audio_system::load_chunk("Test.ogg");
	test_loop_audio = egkr::audio::audio_system::load_chunk("Fire_loop.ogg");
	test_music = egkr::audio::audio_system::load_stream("Woodland Fantasy.ogg");

	test_emitter.audio_file = test_loop_audio;
	test_emitter.looping = true;
	test_emitter.falloff = 1.F;

	egkr::audio::audio_system::set_master_volume(1.F);
	egkr::audio::audio_system::set_channel_volume(0, 1.F);
	egkr::audio::audio_system::set_channel_volume(1, 0.75F);
	egkr::audio::audio_system::set_channel_volume(2, 0.5F);
	egkr::audio::audio_system::set_channel_volume(3, 0.25F);
	egkr::audio::audio_system::set_channel_volume(4, 0.F);
	egkr::audio::audio_system::set_channel_volume(5, 0.F);
	egkr::audio::audio_system::set_channel_volume(6, 1.F);
	egkr::audio::audio_system::set_channel_volume(7, 1.0F);

	egkr::audio::audio_system::play_emitter(6, &test_emitter);
	egkr::audio::audio_system::play_channel(7, test_music, true);

	egkr::debug_console::create();
	egkr::debug_console::load();

	return true;
}

void sandbox_application::update(const egkr::frame_data& frame_data)
{
	//box_->set_colour((egkr::light_system::get_point_lights()[0].colour));

	main_scene_->update(frame_data, camera_, (float)width_ / height_);

	egkr::audio::audio_system::set_listener_orientation(camera_->get_position(), camera_->get_forward(), camera_->get_up());

	egkr::debug_console::update();
}

void sandbox_application::render(egkr::render_packet* render_packet, const egkr::frame_data& frame_data)
{
	if (main_scene_->is_loaded())
	{
		auto& model = meshes_[0]->model();
		glm::quat q({ 0, 0, 0.5F * frame_data.delta_time });
		model.rotate(q);

		auto& model_2 = meshes_[1]->model();
		model_2.rotate(q);

	}
	main_scene_->populate_render_packet(render_packet);

	auto cam = egkr::camera_system::get_default();
	const auto& pos = cam->get_position();
	std::string text = std::format("Camera pos: {} {} {}\n {} meshes drawn", pos.x, pos.y, pos.z, application_frame_data.world_geometries.size());

	more_test_text_->set_text(text);

	egkr::vector<egkr::text::ui_text::weak_ptr> texts{ test_text_, more_test_text_ };
	if (egkr::debug_console::is_visible())
	{
		texts.push_back(egkr::debug_console::get_text());
		texts.push_back(egkr::debug_console::get_entry_text());
	}
	egkr::ui_packet_data ui{ .mesh_data = {ui_meshes_}, .texts = texts };
	auto ui_view = egkr::view_system::get("ui");
	render_packet->render_views[egkr::render_view::type::ui] = egkr::view_system::build_packet(ui_view.get(), &ui);

	application_frame_data.reset();
}

bool sandbox_application::resize(uint32_t width, uint32_t height)
{
	if (width_ != width || height_ != height)
	{
		width_ = width;
		height_ = height;
		return true;
	}

	return false;
}

bool sandbox_application::boot()
{
	{
		egkr::renderpass::configuration renderpass_configuration
		{
			.name = "Renderpass.Builtin.Skybox",
			.render_area = {0, 0, 800, 600},
			.clear_colour = {0, 0, 0.2F, 1.F},
			.clear_flags = egkr::renderpass::clear_flags::colour,
			.depth = 1.F,
			.stencil = 0
		};

		egkr::render_target::attachment_configuration attachment_configration
		{
			.type = egkr::render_target::attachment_type::colour,
			.source = egkr::render_target::attachment_source::default_source,
			.load_operation = egkr::render_target::load_operation::dont_care,
			.store_operation = egkr::render_target::store_operation::store,
			.present_after = false
		};

		renderpass_configuration.target.attachments.push_back(attachment_configration);

		egkr::render_view::configuration skybox_world{};
		skybox_world.type = egkr::render_view::type::skybox;
		skybox_world.width = width_;
		skybox_world.height = height_;
		skybox_world.name = "skybox";
		skybox_world.passes.push_back(renderpass_configuration);
		skybox_world.view_source = egkr::render_view::view_matrix_source::scene_camera;
		render_view_configuration_.push_back(skybox_world);
	}
	{
		egkr::renderpass::configuration renderpass_configuration
		{
			.name = "Renderpass.Builtin.World",
			.render_area = {0, 0, 800, 600},
			.clear_colour = {0, 0, 0.2F, 1.F},
			.clear_flags = egkr::renderpass::clear_flags::depth | egkr::renderpass::clear_flags::stencil,
			.depth = 1.F,
			.stencil = 0
		};

		egkr::render_target::attachment_configuration colour_attachment_configration
		{
			.type = egkr::render_target::attachment_type::colour,
			.source = egkr::render_target::attachment_source::default_source,
			.load_operation = egkr::render_target::load_operation::load,
			.store_operation = egkr::render_target::store_operation::store,
			.present_after = false
		};

	
		egkr::render_target::attachment_configuration depth_attachment_configration
		{
			.type = egkr::render_target::attachment_type::depth,
			.source = egkr::render_target::attachment_source::default_source,
			.load_operation = egkr::render_target::load_operation::dont_care,
			.store_operation = egkr::render_target::store_operation::store,
			.present_after = false
		};

		renderpass_configuration.target.attachments.push_back(colour_attachment_configration);
		renderpass_configuration.target.attachments.push_back(depth_attachment_configration);

		egkr::render_view::configuration opaque_world{};
		opaque_world.type = egkr::render_view::type::world;
		opaque_world.width = width_;
		opaque_world.height = height_;
		opaque_world.name = "world-opaque";
		opaque_world.passes.push_back(renderpass_configuration);
		opaque_world.view_source = egkr::render_view::view_matrix_source::scene_camera;

		render_view_configuration_.push_back(opaque_world);
	}
	{
		egkr::renderpass::configuration renderpass_configuration
		{
			.name = "Renderpass.Builtin.UI",
			.render_area = {0, 0, 800, 600},
			.clear_colour = {0, 0, 0.2F, 1.F},
			.clear_flags = egkr::renderpass::clear_flags::none,
			.depth = 1.F,
			.stencil = 0
		};

		egkr::render_target::attachment_configuration attachment_configration
		{
			.type = egkr::render_target::attachment_type::colour,
			.source = egkr::render_target::attachment_source::default_source,
			.load_operation = egkr::render_target::load_operation::load,
			.store_operation = egkr::render_target::store_operation::store,
			.present_after = true
		};

		renderpass_configuration.target.attachments.push_back(attachment_configration);

		egkr::render_view::configuration ui{};
		ui.name = "ui";
		ui.width = width_;
		ui.height = height_;
		ui.type = egkr::render_view::type::ui;
		ui.view_source = egkr::render_view::view_matrix_source::ui_camera;
		ui.passes.push_back(renderpass_configuration);
		render_view_configuration_.push_back(ui);
	}

	egkr::setup_keymaps(this);

	return true;
}

bool sandbox_application::shutdown()
{
	egkr::debug_console::shutdown();

	main_scene_->destroy();

	meshes_.clear();
	if (box_)
	{
		box_->unload();
		box_.reset();
	}
	ui_meshes_.clear();
	if (ui_mesh_)
	{
		ui_mesh_.reset();
	}
	if (test_text_)
	{
		test_text_.reset();
	}
	if (more_test_text_)
	{
		more_test_text_.reset();
	}
	//debug_frustum_->destroy();

	return true;
}

bool sandbox_application::on_debug_event(egkr::event::code code, void* /*sender*/, void* listener, const egkr::event::context& /*context*/)
{
	auto* application = (sandbox_application*)listener;
	if (code == egkr::event::code::debug01)
	{
		const std::array<std::string_view, 2> materials{ "Random_Stones", "Seamless" };

		static uint32_t choice = 0;
		choice++;
		choice %= materials.size();

		auto material = egkr::material_system::acquire(materials[choice]);
		application->meshes_[0]->get_geometries()[0]->set_material(material);
		//application->update_frustum_ = !application->update_frustum_;
	}

	if (code == egkr::event::code::debug02)
	{
		if (!application->scene_loaded_)
		{
			LOG_INFO("Loading scene");
			application->scene_loaded_ = true;

			application->load_scene();
			return true;
		}
	}

	if (code == egkr::event::code::debug03)
	{
		if (application->scene_loaded_)
		{
			LOG_INFO("Unloading scene");

			application->main_scene_->unload();
			application->meshes_.clear();
			application->scene_loaded_ = false;

			return true;
		}
	}
	return false;
}

bool sandbox_application::on_event(egkr::event::code code, void* /*sender*/, void* listener, const egkr::event::context& context)
{
	auto* game = (sandbox_application*)listener;
	if (code == egkr::event::code::hover_id_changed)
	{
		context.get(0, game->hovered_object_id_);
	}
	return false;
}

void sandbox_application::load_scene()
{

	const auto scene = egkr::resource_system::load("test_scene.ess", egkr::resource::type::scene, nullptr);
	auto scene_configuration = *(egkr::scene::configuration*)scene->data;


	egkr::skybox::configuration skybox_config{ .name =  scene_configuration.skybox.name};
	skybox_ = egkr::skybox::skybox::create(skybox_config);
	main_scene_->add_skybox(skybox_->get_name(), skybox_);

	{
		egkr::mesh::configuration cube_1{};
		cube_1.geometry_configurations.push_back(egkr::geometry_system::generate_cube(10, 10, 10, 1, 1, "cube_1", "test_material"));

		auto mesh_1 = egkr::mesh::create(cube_1);
		mesh_1->set_model(egkr::transform::create());
		meshes_.push_back(mesh_1);

		main_scene_->add_mesh(mesh_1->get_name(), mesh_1);
	}

	{
		egkr::mesh::configuration cube_2{};
		cube_2.geometry_configurations.push_back(egkr::geometry_system::generate_cube(5, 5, 5, 1, 1, "cube_2", "test_material"));

		auto mesh_2 = egkr::mesh::create(cube_2);
		egkr::transform model = egkr::transform::create({ 10.F, 0.F, 0.F });
		model.set_parent(&meshes_.back()->model());
		mesh_2->set_model(model);
		meshes_.push_back(mesh_2);

		main_scene_->add_mesh(mesh_2->get_name(), mesh_2);
	}

	for (const auto& light : scene_configuration.point_lights)
	{
		egkr::light::point_light lght
		{
			.position = light.position,
			.colour = light.colour,
			.constant = light.constant,
			.linear = light.linear,
			.quadratic = light.quadratic
		};

		main_scene_->add_point_light(light.name, lght);

		box_ = egkr::debug::debug_box3d::create({ 0.2, 0.2, 0.2 }, nullptr);
		box_->get_transform().set_position(light.position);
		box_->set_colour(light.colour);
		main_scene_->add_debug(light.name + "_box", box_);
	}


	//egkr::light::point_light light_1
	//{
	//	.position = {5.5, -5.5, 0.0, 0.0},
	//	.colour = {1.0, 0.0, 0.0, 1.0},
	//	.constant = 1.0F,
	//	.linear = 0.35F,
	//	.quadratic = 0.44F,
	//};
	//main_scene_->add_point_light("light_1", light_1);

	//egkr::light::point_light light_2
	//{
	//.position = {5.5, 5.5, 0.0, 0.0},
	//.colour = {0.0, 0.0, 1.0, 1.0 },
	//.constant = 1.0F,
	//.linear = 0.35F,
	//.quadratic = 0.44F,
	//};
	//main_scene_->add_point_light("light_2", light_2);

	dir_light_ = std::make_shared<egkr::light::directional_light>(
		scene_configuration.directional_light.direction,
		scene_configuration.directional_light.colour
	);
	main_scene_->add_directional_light(scene_configuration.directional_light.name, dir_light_);

	egkr::debug::configuration grid_configuration
	{
		.name = "debug_grid",
		.orientation = egkr::debug::orientation::xy,
		.tile_count_dim0 = 100,
		.tile_count_dim1 = 100,
		.tile_scale = 1,
		.use_third_axis = true,
	};
	grid_ = egkr::debug::debug_grid::create(grid_configuration);

	main_scene_->add_debug(grid_configuration.name, grid_);

	for (const auto& mesh : scene_configuration.meshes)
	{
		auto msh = egkr::mesh::create({ mesh.resource_name });
		msh->set_model(mesh.transform);
		meshes_.push_back(msh);
		main_scene_->add_mesh(mesh.name, msh);

		if (mesh.parent_name)
		{
			for (const auto& m : meshes_)
			{
				if (m->get_name() == mesh.parent_name.value())
				{
					msh->model().set_parent(&m->model());
				}
			}
		}
	}
	//sponza_ = egkr::mesh::create({ "sponza2" });

	//egkr::transform obj = egkr::transform::create({ 0, 0, -5 });
	//obj.set_scale({ 0.1F, 0.1F, 0.1F });
	//obj.set_rotation(glm::quat{ { glm::radians(90.F), 0, 0 } });

	//sponza_->set_model(obj);
	//meshes_.push_back(sponza_);

	//main_scene_->add_mesh("sponza2", sponza_);


	//TODO temp
	main_scene_->load();
}
