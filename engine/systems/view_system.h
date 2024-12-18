#pragma once
#include "pch.h"
#include <renderer/render_view.h>

#include <systems/system.h>

namespace egkr
{
	class view_system : public system
	{
	public:
		using view_reference = uint32_t;
		using unique_ptr = std::unique_ptr<view_system>;

		static view_system* create();
		view_system();
		~view_system() override;

		bool init() override;
		bool shutdown() override;

		static void create_view(const render_view::configuration& configuration);
		static	void on_window_resize(uint32_t width, uint32_t height);
		static render_view::shared_ptr get(std::string_view name);
		static render_view_packet build_packet(render_view* view, void* data, const camera::shared_ptr& camera, viewport* viewport);
		static bool on_render(const render_view* view, render_view_packet* packet, const frame_data& frame_data);

	private:
		uint32_t max_view_count_{};
		egkr::vector<render_view::shared_ptr> registered_views_{};
		std::unordered_map<std::string, view_reference> registered_views_by_name_{};
	};
}
