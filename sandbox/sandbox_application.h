#pragma once

#include "application/application.h"
#include "resources/mesh.h"
#include "resources/ui_text.h"
#include "resources/light.h"
#include <resources/audio.h>
#include "debug/debug_box3d.h"
#include "debug/debug_grid.h"
#include "debug/debug_frustum.h"
#include "debug/debug_console.h"

class sandbox_application final : public egkr::application
{
public:
	explicit sandbox_application(const egkr::engine_configuration& configuration);
	bool init() final;
	void update(double delta_time) final;
	void render(egkr::render_packet* render_packet, double delta_time) final;
	bool resize(uint32_t width, uint32_t height) final;

	bool boot() final;
	bool shutdown() final;

private:	
	bool static on_debug_event(egkr::event_code code, void* sender, void* listener, const egkr::event_context& context);
	bool static on_event(egkr::event_code code, void* sender, void* listener, const egkr::event_context& context);

private:

	egkr::frustum camera_frustum_;
	bool update_frustum_{true};
	egkr::debug::debug_frustum::shared_ptr debug_frustum_{};
	egkr::skybox::skybox::shared_ptr skybox_{};
	egkr::vector<egkr::mesh::shared_ptr> meshes_{};
	egkr::vector<egkr::mesh::shared_ptr> ui_meshes_{};
	egkr::text::ui_text::shared_ptr test_text_{};
	egkr::text::ui_text::shared_ptr more_test_text_{};

	egkr::mesh::shared_ptr sponza_{};
	bool models_loaded_{};
	egkr::debug::debug_box3d::shared_ptr box_{};
	egkr::debug::debug_grid::shared_ptr grid_{};

	std::shared_ptr<egkr::light::directional_light> dir_light_{};

	egkr::geometry::frame_data frame_data{};

	egkr::audio::file* test_audio{};
	egkr::audio::file* test_loop_audio{};
	egkr::audio::file* test_music{};
	egkr::audio::emitter test_emitter{};

	uint32_t hovered_object_id_{};
};