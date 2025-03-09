#pragma once

#include "application/application.h"
#include "renderer/renderer_types.h"
#include "resources/mesh.h"
#include "resources/terrain.h"
#include "resources/ui_text.h"
#include "resources/light.h"
#include <resources/audio.h>
#include "debug/debug_box3d.h"
#include "debug/debug_grid.h"
#include "debug/debug_frustum.h"
#include <scenes/simple_scene.h>
#include <editor_gizmo.h>
#include <renderer/viewport.h>

#include <renderer/render_graph.h>
#include <renderer/passes/skybox_pass.h>
#include <renderer/passes/scene_pass.h>
#include <renderer/passes/editor_pass.h>
#include <renderer/passes/ui_pass.h>

class sandbox_application final : public egkr::application
{
public:
    sandbox_application(const egkr::engine_configuration& configuration, egkr::renderer_backend::unique_ptr renderer_plugin);
    bool init() final;
    void update(const egkr::frame_data& frame_data) final;
    void prepare_frame(const egkr::frame_data& frame_data) final;
    void render_frame(egkr::frame_data& frame_data) final;
    bool resize(uint32_t width, uint32_t height) final;

    bool boot() final;
    bool shutdown() final;

    [[nodiscard]] auto& get_gizmo() { return gizmo_; }
private:
    bool static on_debug_event(egkr::event::code code, void* sender, void* listener, const egkr::event::context& context);
    bool static on_event(egkr::event::code code, void* sender, void* listener, const egkr::event::context& context);
    bool static on_button_up(egkr::event::code code, void* sender, void* listener, const egkr::event::context& context);
    bool static on_mouse_move(egkr::event::code code, void* sender, void* listener, const egkr::event::context& context);
    bool static on_mouse_drag(egkr::event::code code, void* sender, void* listener, const egkr::event::context& context);

    void load_scene();
    void configure_rendergraph();
private:
    egkr::scene::simple_scene::unique_ptr main_scene_;
    egkr::frustum camera_frustum_;
    egkr::skybox::shared_ptr skybox_;
    egkr::terrain::shared_ptr terrain_;
    egkr::vector<egkr::mesh::shared_ptr> meshes_;
    egkr::vector<egkr::mesh::weak_ptr> ui_meshes_;
    egkr::mesh::shared_ptr ui_mesh_;
    egkr::text::ui_text::shared_ptr test_text_;
    egkr::text::ui_text::shared_ptr more_test_text_;

    egkr::mesh::shared_ptr sponza_;
    bool scene_loaded_{};

    egkr::debug::debug_box3d::shared_ptr box_;
    egkr::debug::debug_grid::shared_ptr grid_;
    egkr::debug::debug_frustum::shared_ptr debug_frustum_;
    egkr::debug::debug_line::shared_ptr test_lines_;
    egkr::debug::debug_box3d::shared_ptr test_boxes_;

    std::shared_ptr<egkr::light::directional_light> dir_light_;

    // egkr::audio::file* test_audio{};
    // egkr::audio::file* test_loop_audio{};
    // egkr::audio::file* test_music{};
    egkr::audio::emitter test_emitter{};

    egkr::editor::gizmo gizmo_;
    egkr::int2 mouse_pos_{};

    uint32_t hovered_object_id_{};

    egkr::viewport world_view;
    egkr::viewport world_view2;
    egkr::viewport ui_view;

    egkr::rendergraph frame_graph;
    egkr::pass::skybox* skybox_pass{};
    egkr::pass::scene* scene_pass{};
    egkr::pass::editor* editor_pass{};
    egkr::pass::ui* ui_pass{};
};
