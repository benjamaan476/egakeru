#include "ui_system.h"

namespace egkr
{
	static ui_system::unique_ptr ui_system_state;

	ui_system * ui_system::create(const configuration& configuration)
	{
		ui_system_state = std::make_unique<ui_system>(configuration);
		return ui_system_state.get();
	}

	ui_system::ui_system(const configuration& /*configuration*/)
	{

	}

	bool ui_system::init()
	{

		return true;
	}

	bool ui_system::update(const frame_data& frame_data)
	{
		for (const auto& control : ui_system_state->registered_controls | std::views::filter([](const sui::control::shared_ptr& control) { return control->is_active(); }))
		{
			control->update(frame_data);
		}
		return true;
	}

	bool ui_system::render(const frame_data& frame_data)
	{
		return true;
	}

	bool ui_system::register_control(const sui::control::shared_ptr& control)
	{
		if (ui_system_state->registered_controls_by_name.contains(control->name))
		{
			LOG_ERROR("Control with name {} already registered. Cannot register control with duplicate name");
			return false;
		}

		ui_system_state->registered_controls.push_back(control);
		ui_system_state->registered_controls_by_name[control->name] = ui_system_state->registered_controls.size();

		return true;
	}

	bool ui_system::shutdown()
	{
		return true;
	}
}
