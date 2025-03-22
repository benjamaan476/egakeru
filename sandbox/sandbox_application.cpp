#include "sandbox_application.h"
#include "loaders/resource_loader.h"
#include "log/log.h"
#include "renderer/render_graph.h"
#include "resources/geometry.h"
#include "sandbox_keybinds.h"

#include <iterator>
#include <systems/geometry_system.h>
#include <systems/light_system.h>
#include <systems/camera_system.h>
#include <systems/audio_system.h>
#include <systems/material_system.h>

#include <systems/resource_system.h>

#include <renderer/renderer_types.h>
#include <renderer/renderer_frontend.h>
#include <debug/debug_console.h>
#include <systems/input.h>

#include "ray.h"

sandbox_application::sandbox_application(const egkr::engine_configuration& configuration, egkr::renderer_backend::unique_ptr plugin): application(configuration, std::move(plugin))
{
    egkr::bitmap_font_configuration bitmap_font_configuration{.name = "Arial 32", .size = 32, .resource_name = "Arial32"};
    egkr::system_font_configuration system_font_configuration{.name = "NotoSansCJK", .default_size = 24, .resource_name = "NotoSansCJK"};
    font_system_configuration_ = {.system_font_configurations = {system_font_configuration}, .bitmap_font_configurations = {bitmap_font_configuration}, .max_system_font_count = 1, .max_bitmap_font_count = 1};
}

bool sandbox_application::init()
{
    LOG_INFO("Sandbox application created");

    egkr::event::register_event(egkr::event::code::debug01, this, sandbox_application::on_debug_event);
    egkr::event::register_event(egkr::event::code::debug02, this, sandbox_application::on_debug_event);
    egkr::event::register_event(egkr::event::code::debug03, this, sandbox_application::on_debug_event);
    egkr::event::register_event(egkr::event::code::mouse_up, this, sandbox_application::on_button_up);
    egkr::event::register_event(egkr::event::code::mouse_drag, this, sandbox_application::on_mouse_drag);
    egkr::event::register_event(egkr::event::code::mouse_drag_begin, this, sandbox_application::on_mouse_drag);
    egkr::event::register_event(egkr::event::code::mouse_drag_end, this, sandbox_application::on_mouse_drag);
    egkr::event::register_event(egkr::event::code::mouse_move, this, sandbox_application::on_mouse_move);

    test_text_ = egkr::text::ui_text::create(egkr::text::type::bitmap, "Arial 32", 32, "some test text! \n\t mah~`?^");
    test_text_->set_position({50, 250, 0});

    more_test_text_ = egkr::text::ui_text::create(egkr::text::type::bitmap, "Arial 32", 32, "a");
    more_test_text_->set_position({50, 400, 0});

    main_scene_ = egkr::scene::simple_scene::create({});

    std::vector<vertex_2d> vertices{4};

    vertices[0] = {{0.F, 136.F}, {0.F, 1.F}};
    vertices[1] = {{512.F, 136.F}, {1.F, 1.F}};
    vertices[2] = {{512.F, 0.F}, {1.F, 0.F}};
    vertices[3] = {{0.F, 0.F}, {0.F, 0.F}};

    std::vector<uint32_t> indices{0, 1, 2, 0, 2, 3};

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

    camera_ = egkr::camera_system::acquire("world");
    camera_->set_position({10, 10, 10});
    camera_->set_rotation({0.f, glm::radians(45.f), glm::radians(45.f)});
    camera_->set_aspect((float)width_ / (float)height_);

    auto cam = egkr::camera_system::get_default();
    cam->set_position({10, 10, 10});
    cam->set_rotation({0.f, glm::radians(45.f), glm::radians(45.f)});
    cam->set_aspect((float)width_ / (float)height_);

    //TODO add to scene
    // test_audio = egkr::audio::audio_system::load_chunk("Test.ogg");
    // test_loop_audio = egkr::audio::audio_system::load_chunk("Fire_loop.ogg");
    // test_music = egkr::audio::audio_system::load_stream("Woodland Fantasy.ogg");

    // test_emitter.audio_file = test_loop_audio;
    // test_emitter.looping = true;
    // test_emitter.falloff = 1.F;

    // egkr::audio::audio_system::set_master_volume(1.F);
    // egkr::audio::audio_system::set_channel_volume(0, 1.F);
    // egkr::audio::audio_system::set_channel_volume(1, 0.75F);
    // egkr::audio::audio_system::set_channel_volume(2, 0.5F);
    // egkr::audio::audio_system::set_channel_volume(3, 0.25F);
    // egkr::audio::audio_system::set_channel_volume(4, 0.F);
    // egkr::audio::audio_system::set_channel_volume(5, 0.F);
    // egkr::audio::audio_system::set_channel_volume(6, 1.F);
    // egkr::audio::audio_system::set_channel_volume(7, 1.0F);

    //egkr::audio::audio_system::play_emitter(6, &test_emitter);
    //egkr::audio::audio_system::play_channel(7, test_music, true);

    egkr::debug_console::create();
    egkr::debug_console::load();

    gizmo_ = egkr::editor::gizmo::create();
    gizmo_.init();
    gizmo_.load();

    world_view = egkr::viewport::create({0, 0, width_, height_}, egkr::projection_type::perspective, glm::radians(57.f), 0.1f, 4000.f);
    world_view2 = egkr::viewport::create({20, 20, 128.8, 72}, egkr::projection_type::perspective, glm::radians(57.f), 0.1f, 4000.f);
    ui_view = egkr::viewport::create({0, 0, width_, height_}, egkr::projection_type::orthographic, 0, -100.f, 100.f);


    frame_graph.load_resources();
    return true;
}

void sandbox_application::update(const egkr::frame_data& frame_data)
{
    //box_->set_colour((egkr::light_system::get_point_lights()[0].colour));

    main_scene_->update(frame_data, camera_, &world_view);

    // egkr::audio::audio_system::set_listener_orientation(camera_->get_position(), camera_->get_forward(), camera_->get_up());

    egkr::debug_console::update();
}

void sandbox_application::prepare_frame(const egkr::frame_data& /*frame_data*/)
{
    skybox_pass->view = camera_->get_view();
    skybox_pass->projection = world_view.projection;
    skybox_pass->view_position = camera_->get_position();
    skybox_pass->do_execute = true;
    skybox_pass->viewport_ = &world_view;
    if (main_scene_->is_loaded())
    {
	skybox_pass->data.skybox_data = main_scene_->get_skybox();
	scene_pass->data.irradiance_texture = main_scene_->get_skybox()->get_texture_map()->map_texture;
    }
    else
    {
	skybox_pass->data.skybox_data = nullptr;
    }

    const auto& frame_data = main_scene_->get_frame_data();
    shadow_pass->do_execute = true;
    shadow_pass->data.geometries = frame_data.world_geometries;
    shadow_pass->data.terrain = frame_data.terrain_geometries;

    scene_pass->do_execute = true;
    scene_pass->viewport_ = &world_view;
    scene_pass->view = camera_->get_view();
    scene_pass->projection = world_view.projection;
    scene_pass->view_position = camera_->get_position();

    scene_pass->data.geometries = frame_data.world_geometries;
    scene_pass->data.debug_geometries = frame_data.debug_geometries;
    scene_pass->data.terrain = frame_data.terrain_geometries;
    scene_pass->data.ambient_colour = main_scene_->get_ambient_colour();

    if(auto dir_light = main_scene_->get_directional_light())
    {
	const egkr::float4 light_dir = glm::normalize(dir_light->direction);
	egkr::float3 shadow_map_cam_pos = -100.f * light_dir;
	auto camera_looat = glm::lookAt(shadow_map_cam_pos, egkr::float3(0), {0,0,1});
	scene_pass->data.directional_light_view = camera_looat;
    }
    if (test_lines_)
    {
	scene_pass->data.debug_geometries.push_back(egkr::render_data{.render_geometry = test_lines_->get_geometry(), .transform = test_lines_});
    }
    

    const auto& pos = camera_->get_position();
    std::string text = std::format("Camera pos: {:.3} {:.3} {:.3}\n Mouse pos: {} {}", (double)pos.x, (double)pos.y, (double)pos.z, mouse_pos_.x, mouse_pos_.y);

    more_test_text_->set_text(text);

    egkr::vector<egkr::text::ui_text::weak_ptr> texts{test_text_, more_test_text_};
    if (egkr::debug_console::is_visible())
    {
	texts.push_back(egkr::debug_console::get_text());
	texts.push_back(egkr::debug_console::get_entry_text());
    }
    ui_pass->data.mesh_data = ui_meshes_;
    egkr::render_data data{.render_geometry = ui_mesh_->get_geometries()[0], .transform = ui_mesh_, .is_winding_reversed = ui_mesh_->get_determinant() < 0.f};

    ui_pass->data.ui_geometries.push_back(data);
    ui_pass->data.texts = texts;
    ui_pass->viewport_ = &ui_view;
    ui_pass->projection = ui_view.projection;
    ui_pass->do_execute = true;
    ui_pass->view = glm::mat4(1.F);

    editor_pass->do_execute = true;
    editor_pass->viewport_ = &world_view;
    editor_pass->view = camera_->get_view();
    editor_pass->view_position = camera_->get_position();
    editor_pass->projection = world_view.projection;

    editor_pass->data.gizmo = gizmo_;
}

void sandbox_application::render_frame(egkr::frame_data& frame_data)
{
    const auto& renderer = get_renderer();
    renderer->prepare_frame(frame_data);
    renderer->begin(frame_data);
    frame_graph.execute(frame_data);
    renderer->end(frame_data);
    renderer->present(frame_data);
}

bool sandbox_application::resize(uint32_t width, uint32_t height)
{
    if (width_ != width || height_ != height)
    {
	width_ = width;
	height_ = height;

	if (!width || !height)
	{
	    return false;
	}
	float half_width = (float)width / 2.f;
	world_view.resize({0, 0, width, height});
	ui_view.resize({0, 0, width, height});
	world_view2.resize({0, 0, half_width, height});
	camera_->set_aspect((float)width / (float)height);
	return true;
    }

    return false;
}


bool sandbox_application::boot()
{
    configure_rendergraph();
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
    if (test_lines_)
    {
	test_lines_.reset();
    }
    gizmo_.unload();
    gizmo_.destroy();

    frame_graph.destroy();

    //skybox_->destroy();
    //debug_frustum_->destroy();

    // egkr::audio::audio_system::close(test_audio);
    // egkr::audio::audio_system::close(test_loop_audio);
    // egkr::audio::audio_system::close(test_music);

    return true;
}

bool sandbox_application::on_debug_event(egkr::event::code code, void* /*sender*/, void* listener, const egkr::event::context& /*context*/)
{
    auto* application = (sandbox_application*)listener;
    if (code == egkr::event::code::debug01)
    {
	const std::array<std::string, 2> materials{"random_stones", "seamless"};

	static uint32_t choice = 0;
	choice++;
	choice %= materials.size();

	auto material = egkr::material_system::acquire(materials[choice]);
	application->meshes_[0]->get_geometries()[0]->set_material(material);
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

bool sandbox_application::on_button_up(egkr::event::code code, void* /*sender*/, void* listener, const egkr::event::context& context)
{
    auto* game = (sandbox_application*)listener;

    if (code == egkr::event::code::mouse_up)
    {
	int32_t button{};
	context.get(0, button);

	int32_t xpos{};
	context.get(1, xpos);

	int32_t ypos{};
	context.get(2, ypos);

	switch ((egkr::mouse_button)(button))
	{
	case egkr::mouse_button::left:
	{
	    const auto& view = game->camera_->get_view();
	    const auto& origin = game->camera_->get_position();
	    const auto& proj = game->world_view.projection;
	    egkr::ray ray = egkr::ray::from_screen({xpos, ypos}, game->world_view.viewport_rect, origin, view, proj);

	    auto hit = game->gizmo_.raycast(ray);
	    if (!hit)
	    {
		hit = game->main_scene_->raycast(ray);
	    }
	    if (hit)
	    {
		LOG_INFO("Hit! position: {}, distance: {} id: {}", glm::to_string(hit.hits.front().position), hit.hits.front().distance, hit.hits.front().unique_id);
		egkr::debug::debug_line::shared_ptr ray_line = egkr::debug::debug_line::create(ray.origin, ray.origin + 100.f * ray.direction);
		ray_line->set_colour(0, {1, 0, 1, 1});
		ray_line->set_colour(1, {0, 1, 1, 1});
		game->test_lines_ = ray_line;

		if (const auto& selected = game->main_scene_->get_transform(hit.hits.front().unique_id))
		{
		    game->gizmo_.set_selected(selected);
		}
	    }
	    break;
	}
	default:
	    break;
	}
    }
    return false;
}

bool sandbox_application::on_mouse_move(egkr::event::code code, void* /*sender*/, void* listener, const egkr::event::context& context)
{
    if (code == egkr::event::code::mouse_move)
    {
	auto* game = (sandbox_application*)listener;
	egkr::int2 pos;
	context.get(0, pos.x);
	context.get(1, pos.y);
	game->mouse_pos_ = pos;

	if (!egkr::input::is_button_dragging(egkr::mouse_button::left))
	{
	    auto& camera = game->camera_;
	    const auto& view = camera->get_view();
	    const auto& origin = camera->get_position();

	    egkr::ray ray = egkr::ray::from_screen(pos, game->world_view.viewport_rect, origin, view, game->world_view.projection);

	    game->gizmo_.handle_interaction(egkr::editor::gizmo::interaction_type::mouse_hover, ray);
	}
    }

    return false;
}

bool sandbox_application::on_mouse_drag(egkr::event::code code, void* /*sender*/, void* listener, const egkr::event::context& context)
{
    auto* game = (sandbox_application*)listener;

    egkr::int2 pos;
    context.get(0, pos.x);
    context.get(1, pos.y);
    game->mouse_pos_ = pos;

    auto& camera = game->camera_;
    const auto& view = camera->get_view();
    const auto& origin = camera->get_position();


    egkr::ray ray = egkr::ray::from_screen(pos, game->world_view.viewport_rect, origin, view, game->world_view.projection);

    if (code == egkr::event::code::mouse_drag_begin)
    {
	game->gizmo_.begin_interaction(egkr::editor::gizmo::interaction_type::mouse_drag, ray);
    }
    else if (code == egkr::event::code::mouse_drag)
    {
	game->gizmo_.handle_interaction(egkr::editor::gizmo::interaction_type::mouse_drag, ray);
    }
    else if (code == egkr::event::code::mouse_drag_end)
    {
	game->gizmo_.end_interaction(egkr::editor::gizmo::interaction_type::mouse_drag, ray);
    }
    return false;
}

void sandbox_application::load_scene()
{
    const auto scene = egkr::resource_system::load("test_scene.ess", egkr::resource::type::scene, nullptr);
    auto scene_configuration = *(egkr::scene::configuration*)scene->data;

    egkr::skybox::configuration skybox_config{.name = scene_configuration.skybox.name};
    skybox_ = egkr::skybox::skybox::create(skybox_config);
    main_scene_->add_skybox(skybox_->get_name(), skybox_);

    for (const auto& terrain_config : scene_configuration.terrains)
    {
	if (terrain_config.resource_name.empty())
	{
	    LOG_ERROR("Invalid terrain specified, terrain must contain a resource_name in scene description");
	    continue;
	}

	auto terrain_resource = egkr::resource_system::load(terrain_config.resource_name, egkr::resource::type::terrain, nullptr);
	if (!terrain_resource)
	{
	    LOG_ERROR("Couldn't load terrain resource for {}", terrain_config.resource_name);
	    continue;
	}

	auto terrain = egkr::terrain::create(*(egkr::terrain::configuration*)terrain_resource->data);
	main_scene_->add_terrain(terrain_config.name, terrain);
    }

    {
	egkr::mesh::configuration cube_1{};
	cube_1.geometry_configurations.push_back(egkr::geometry_system::generate_cube(10, 10, 10, 1, 1, "cube_1", "test_material"));

	auto mesh_1 = egkr::mesh::create(cube_1);
	mesh_1->set_name("test_cube_1");
	meshes_.push_back(mesh_1);

	main_scene_->add_mesh(mesh_1->get_name(), mesh_1);
    }

    {
	egkr::mesh::configuration cube_2{};
	cube_2.geometry_configurations.push_back(egkr::geometry_system::generate_cube(5, 5, 5, 1, 1, "cube_2", "test_material"));

	auto mesh_2 = egkr::mesh::create(cube_2);
	mesh_2->set_name("test_cube_12");
	mesh_2->set_position({10.f, 0.f, 0.f});
	mesh_2->set_parent(meshes_.back());
	meshes_.push_back(mesh_2);

	main_scene_->add_mesh(mesh_2->get_name(), mesh_2);
    }

    for (const auto& light : scene_configuration.point_lights)
    {
	egkr::light::point_light lght{.position = light.position, .colour = light.colour, .constant = light.constant, .linear = light.linear, .quadratic = light.quadratic};

	main_scene_->add_point_light(light.name, lght);

	box_ = egkr::debug::debug_box3d::create({0.2, 0.2, 0.2}, nullptr);
	box_->set_position(light.position);
	box_->set_colour(light.colour);
	main_scene_->add_debug(light.name + "_box", box_);
    }

    dir_light_ = std::make_shared<egkr::light::directional_light>(scene_configuration.directional_light.direction, scene_configuration.directional_light.colour);
    main_scene_->add_directional_light(scene_configuration.directional_light.name, dir_light_);

    egkr::debug::configuration grid_configuration{
        .name = "debug_grid",
        .grid_orientation = egkr::debug::orientation::xy,
        .tile_count_dim0 = 100,
        .tile_count_dim1 = 100,
        .tile_scale = 1,
        .use_third_axis = true,
    };
    grid_ = egkr::debug::debug_grid::create(grid_configuration);

    main_scene_->add_debug(grid_configuration.name, grid_);

    for (const auto& mesh : scene_configuration.meshes)
    {
	auto msh = egkr::mesh::create({.name = mesh.resource_name});
	msh->set_position(mesh.pos);
	msh->set_rotation({mesh.euler_angles});
	msh->set_scale(mesh.scale);
	meshes_.push_back(msh);
	main_scene_->add_mesh(mesh.name, msh);

	if (mesh.parent_name)
	{
	    for (const auto& m : meshes_)
	    {
		if (m->get_name() == mesh.parent_name.value())
		{
		    msh->set_parent(m);
		}
	    }
	}
    }
    //TODO: temp
    main_scene_->load();
    egkr::resource_system::unload(scene);
}

void sandbox_application::configure_rendergraph()
{
    frame_graph = egkr::rendergraph::create("sandbox");
    frame_graph.add_gloabl_source("colourbuffer", egkr::rendergraph::source::type::render_target_colour, egkr::rendergraph::source::origin::global);
    frame_graph.add_gloabl_source("depthbuffer", egkr::rendergraph::source::type::render_target_depth_stencil, egkr::rendergraph::source::origin::global);

    skybox_pass = (egkr::pass::skybox*)frame_graph.create_pass("skybox", &egkr::pass::skybox::create);
    frame_graph.add_sink("skybox", "colourbuffer");
    frame_graph.add_source("skybox", "colourbuffer", egkr::rendergraph::source::type::render_target_colour, egkr::rendergraph::source::origin::other);
    frame_graph.set_sink_linkage("skybox", "colourbuffer", "", "colourbuffer");

    shadow_pass = (egkr::pass::shadow*)frame_graph.create_pass("shadow", &egkr::pass::shadow::create);
    frame_graph.add_source("shadow", "colourbuffer", egkr::rendergraph::source::type::render_target_colour, egkr::rendergraph::source::origin::self);
    frame_graph.add_source("shadow", "depthbuffer", egkr::rendergraph::source::type::render_target_depth_stencil, egkr::rendergraph::source::origin::self);

    scene_pass = (egkr::pass::scene*)frame_graph.create_pass("scene", &egkr::pass::scene::create);
    frame_graph.add_sink("scene", "colourbuffer");
    frame_graph.add_sink("scene", "depthbuffer");
    frame_graph.add_sink("scene", "shadowmap");
    frame_graph.add_source("scene", "colourbuffer", egkr::rendergraph::source::type::render_target_colour, egkr::rendergraph::source::origin::other);
    frame_graph.add_source("scene", "depthbuffer", egkr::rendergraph::source::type::render_target_depth_stencil, egkr::rendergraph::source::origin::global);
    frame_graph.set_sink_linkage("scene", "colourbuffer", "skybox", "colourbuffer");
    frame_graph.set_sink_linkage("scene", "depthbuffer", {}, "depthbuffer");
    frame_graph.set_sink_linkage("scene", "shadowmap", "shadow", "depthbuffer");

    editor_pass = (egkr::pass::editor*)frame_graph.create_pass("editor", &egkr::pass::editor::create);
    frame_graph.add_sink("editor", "colourbuffer");
    frame_graph.add_sink("editor", "depthbuffer");
    frame_graph.add_source("editor", "colourbuffer", egkr::rendergraph::source::type::render_target_colour, egkr::rendergraph::source::origin::other);
    frame_graph.add_source("editor", "depthbuffer", egkr::rendergraph::source::type::render_target_depth_stencil, egkr::rendergraph::source::origin::other);
    frame_graph.set_sink_linkage("editor", "colourbuffer", "scene", "colourbuffer");
    frame_graph.set_sink_linkage("editor", "depthbuffer", "scene", "depthbuffer");

    ui_pass = (egkr::pass::ui*)frame_graph.create_pass("ui", &egkr::pass::ui::create);
    frame_graph.add_sink("ui", "colourbuffer");
    frame_graph.add_source("ui", "colourbuffer", egkr::rendergraph::source::type::render_target_colour, egkr::rendergraph::source::origin::other);
    frame_graph.set_sink_linkage("ui", "colourbuffer", "editor", "colourbuffer");

    frame_graph.finalise();
}
