#include "system.h"
#include <input.h>

namespace egkr
{
	static std::unique_ptr<system_manager> system_manager_state{};

	void system_manager::create()
	{
		if (system_manager_state)
		{
			LOG_WARN("System manager already initialised");
			return;
		}
		system_manager_state = std::make_unique<system_manager>();
	}
	bool system_manager::init()
	{
		if (!system_manager_state)
		{
			LOG_ERROR("System manager not created. Failed to init");
			return false;
		}
		system_manager_state->registered_systems_.emplace(system_type::input, input::create());
		return true;
	}
	
	bool system_manager::update(float delta_time)
	{
		for (auto& [type, system] : system_manager_state->registered_systems_)
		{
			if (type == system_type::input)
			{
				continue;
			}

			if (!system->update(delta_time))
			{
				return false;
			}
		}
		return true;
	}

	void system_manager::update_input()
	{
		system_manager_state->registered_systems_[system_type::input]->update(0.F);
	}

	void system_manager::shutdown()
	{
		shutdown_user();
		shutdown_extension();
		shutdown_known();

		if (system_manager_state)
		{
			system_manager_state.reset();
		}
	}

	void system_manager::shutdown_extension()
	{ 
	}

	void system_manager::shutdown_user()
	{

	}

	void system_manager::shutdown_known()
	{
		if (system_manager_state)
		{
			for (auto& system : system_manager_state->registered_systems_ | std::views::values)
			{
				system->shutdown();
			}
		}
	}
}