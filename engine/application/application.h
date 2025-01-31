#pragma once
#include "pch.h"
#include "keymap.h"
#include "engine/engine_configuration.h"
#include "renderer/camera.h"
#include "renderer/renderer_frontend.h"
#include "renderer/renderer_types.h"

namespace egkr
{
    class engine;
    struct render_packet;

    class application
    {
    public:
	using unique_ptr = std::unique_ptr<application>;
	API application(engine_configuration configuration, renderer_backend::unique_ptr renderer);

	API virtual ~application() = default;

	virtual bool init() = 0;
	virtual void update(const frame_data& frame_data) = 0;
	virtual void prepare_frame(const frame_data& frame_data) = 0;
	virtual void render_frame(egkr::frame_data& frame_data) = 0;
	virtual bool resize(uint32_t width, uint32_t height) = 0;

	virtual bool boot() = 0;
	virtual bool shutdown() = 0;

	[[nodiscard]] const auto& get_engine_configuration() const { return engine_configuration_; }
	[[nodiscard]] const auto& get_font_system_configuration() const { return font_system_configuration_; }
	[[nodiscard]] const auto& get_camera() const { return camera_; }

	[[nodiscard]] const auto& get_console_keymap() const { return console_keymap; }
	[[nodiscard]] auto& get_console_keymap() { return console_keymap; }

	void set_engine(engine* engine);

	[[nodiscard]] const renderer_frontend::unique_ptr& get_renderer() const;
	[[nodiscard]] renderer_backend::unique_ptr&& move_renderer_plugin() { return std::move(renderer_plugin); }
    protected:
	[[nodiscard]] auto* get_engine() const { return engine_; }
	font_system::configuration font_system_configuration_{};

	engine* engine_;
	uint32_t width_{};
	uint32_t height_{};

	egkr::camera::shared_ptr camera_;
	keymap console_keymap;
    private:
	engine_configuration engine_configuration_{};
	renderer_backend::unique_ptr renderer_plugin;
    };
}
