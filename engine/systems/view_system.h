#pragma once
#include "pch.h"
#include <renderer/render_view.h>

#include "renderer/renderer_frontend.h"

namespace egkr
{
	class view_system
	{
	public:
		using view_reference = uint32_t;
		using unique_ptr = std::unique_ptr<view_system>;

		static bool create(const renderer_frontend* renderer);
		view_system(const renderer_frontend* renderer);
		~view_system();

		static bool init();
		static bool shutdown();

		static void create_view(const render_view::configuration& configuration);
	static	void on_window_resize(uint32_t width, uint32_t height);
	static render_view::render_view::shared_ptr get(std::string_view name);
	static render_view::render_view_packet build_packet(render_view::render_view* view, void* data);
	static bool on_render(const render_view::render_view* view, const render_view::render_view_packet* packet, uint32_t frame_number, uint32_t render_target_index);

	private:
		const renderer_frontend* renderer_{};
		uint32_t max_view_count_{};
		egkr::vector<render_view::render_view::shared_ptr> registered_views_{};
		std::unordered_map<std::string, view_reference> registered_views_by_name_{};
};
}
