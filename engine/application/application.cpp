#include "application.h"

namespace egkr
{
	application::shared_ptr application::create(const application_configuration& config) 
	{
		if (!is_initialised_)
		{
			return std::make_shared<application>(config);
		}

		LOG_WARN("Application already initialised");
		return nullptr;
	}

	application::application(const application_configuration& config)
	{
		state_.width = config.width;
		state_.height = config.height;

		//Init subsystems

		//
		const uint32_t start_x = 100;
		const uint32_t start_y = 100;

		const platform_configuration platform_config = { .start_x = start_x, .start_y = start_y, .width = state_.width, .height = state_.height, .name = config.name };


		state_.platform = egkr::platform::create(egkr::platform_type::windows);
		if (state_.platform == nullptr)
		{
			LOG_FATAL("Failed to create platform");
			return;
		}

		auto success = state_.platform->startup(platform_config);
		if (!success)
		{
			LOG_FATAL("Failed to start platform");
			return;
		}
	}

	void application::run() const
	{
		while (state_.platform->is_running())
		{
			state_.platform->pump();
		}
	}
}