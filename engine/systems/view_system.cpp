#include "view_system.h"

#include <renderer/views/render_view_ui.h>
#include <renderer/views/render_view_world.h>
#include <renderer/views/render_view_skybox.h>

namespace egkr
{
	static view_system::unique_ptr view_system_{};

	view_system* view_system::create()
	{
		view_system_ = std::make_unique<view_system>();
		return view_system_.get();
	}

	view_system::view_system()
		: max_view_count_{ 31 }
	{}

	view_system::~view_system()
	{
	}

	bool view_system::init()
	{
		return true;
	}

	bool view_system::shutdown()
	{
		if (view_system_)
		{
			for (auto& view : view_system_->registered_views_)
			{
				view->on_destroy();
				view.reset();
			}
			view_system_->registered_views_.clear();
			view_system_->registered_views_by_name_.clear();
			view_system_.reset();
		}
		return true;
	}

	void view_system::create_view(const render_view::configuration& configuration)
	{
		if (view_system_->registered_views_by_name_.contains(configuration.name))
		{
			LOG_WARN("View already created");
			return;
		}

		if (view_system_->registered_views_.size() == view_system_->max_view_count_)
		{
			LOG_ERROR("Exceeded view limit of {}", view_system_->max_view_count_);
			return;
		}

		if (configuration.passes.size() < 1)
		{
			LOG_ERROR("View needs at least one renderpass, provided {}", configuration.passes.size());
			return;
		}

		auto render_view = render_view::render_view::create(configuration);

		if (!render_view->on_create())
		{
			LOG_ERROR("Failed to create view {}", configuration.name);
			return;
		}

		view_system_->registered_views_.push_back(render_view);
		view_system_->registered_views_by_name_[configuration.name] = (uint32_t)view_system_->registered_views_.size() - 1;

	}

	void view_system::on_window_resize(uint32_t width, uint32_t height)
	{
		for (auto& view : view_system_->registered_views_)
		{
			view->on_resize(width, height);
		}
	}

	render_view::render_view::shared_ptr view_system::get(std::string_view name)
	{
		if (view_system_->registered_views_by_name_.contains(name.data()))
		{
			auto handle = view_system_->registered_views_by_name_[name.data()];
			return view_system_->registered_views_[handle];
		}

		LOG_ERROR("Attempted to get unregistered view");
		return nullptr;
	}

	render_view_packet view_system::build_packet(render_view* view, void* data)
	{
		return	view->on_build_packet(data);
	}

	bool view_system::on_render(const render_view* view, const render_view_packet* packet, uint32_t frame_number, uint32_t render_target_index)
	{
		return view->on_render(packet, frame_number, render_target_index);
	}
}
