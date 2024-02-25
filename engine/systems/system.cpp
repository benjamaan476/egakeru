#include "system.h"
#include <input.h>

#include <systems/resource_system.h>
#include <systems/texture_system.h>


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

	system_manager::system_manager()
	{
		register_known();
		register_extension();
		register_user();
	}

	bool system_manager::init()
	{
		if (!system_manager_state)
		{
			LOG_ERROR("System manager not created. Failed to init");
			return false;
		}

		for (auto system : system_manager_state->registered_systems_ | std::views::values)
		{
			if (!system->init())
			{
				return false;
			}
		}
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

	void system_manager::register_known()
	{
		registered_systems_.emplace(system_type::input, input::create());
		{
			resource_system_configuration resource_system_configuration{};
			resource_system_configuration.max_loader_count = 10;
			resource_system_configuration.base_path = "../../../../assets/";

			registered_systems_.emplace(system_type::resource, resource_system::create(resource_system_configuration));
		}
		{
			registered_systems_.emplace(system_type::texture, texture_system::create({ 1024 }));
		}
	}

	void system_manager::register_extension()
	{
	}

	void system_manager::register_user()
	{
	}

	void system_manager::shutdown_extension()
	{ 
	}

	void system_manager::shutdown_user()
	{

	}

	void system_manager::shutdown_known()
	{
		if (!system_manager_state)
		{
			return;
		}
		system_manager_state->registered_systems_[system_type::texture]->shutdown();
		system_manager_state->registered_systems_[system_type::resource]->shutdown();
		system_manager_state->registered_systems_[system_type::input]->shutdown();
	}
}