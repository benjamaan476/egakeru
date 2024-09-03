#pragma once

#include "application/application.h"
#include "resources/mesh.h"
#include "resources/ui_text.h"
#include "resources/light.h"
#include <resources/audio.h>
#include "debug/debug_box3d.h"
#include "debug/debug_grid.h"
#include "debug/debug_frustum.h"
#include <scenes/simple_scene.h>
#include <editor_gizmo.h>
#include <renderer/viewport.h>

class sandbox_application final : public egkr::application
{
public:
	explicit sandbox_application(const egkr::engine_configuration& configuration);
	bool init() final;
	void update(const egkr::frame_data& frame_data) final;
	void prepare_render_packet(egkr::render_packet* render_packet, const egkr::frame_data& frame_data) final;
	void render(egkr::render_packet* render_packet, egkr::frame_data& frame_data) final;
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

private:

	egkr::scene::simple_scene::unique_ptr main_scene_{};
	egkr::frustum camera_frustum_;
	bool update_frustum_{true};
	egkr::skybox::shared_ptr skybox_{};
	egkr::vector<egkr::mesh::shared_ptr> meshes_{};
	egkr::vector<egkr::mesh::weak_ptr> ui_meshes_{};
	egkr::mesh::shared_ptr ui_mesh_{};
	egkr::text::ui_text::shared_ptr test_text_{};
	egkr::text::ui_text::shared_ptr more_test_text_{};

	egkr::mesh::shared_ptr sponza_{};
	bool scene_loaded_{};

	egkr::debug::debug_box3d::shared_ptr box_{};
	egkr::debug::debug_grid::shared_ptr grid_{};
	egkr::debug::debug_frustum::shared_ptr debug_frustum_{};
	egkr::debug::debug_line::shared_ptr test_lines_{};
	egkr::debug::debug_box3d::shared_ptr test_boxes_{};

	std::shared_ptr<egkr::light::directional_light> dir_light_{};

	egkr::application_data application_frame_data{};

	egkr::audio::file* test_audio{};
	egkr::audio::file* test_loop_audio{};
	egkr::audio::file* test_music{};
	egkr::audio::emitter test_emitter{};

	egkr::editor::gizmo gizmo_{};
	egkr::int2 mouse_pos_{};

	uint32_t hovered_object_id_{};

	egkr::viewport world_view;
	egkr::viewport world_view2;
	egkr::viewport ui_view;
};
