#include "evar_system.h"
#include "event.h"

namespace egkr
{
	static evar_system::unique_ptr state;

	evar_system* evar_system::create()
	{
		state = std::make_unique<evar_system>();
		return state.get();
	}

	bool evar_system::init()
	{
		return true;
	}

	bool evar_system::update(float /*delta_time*/)
	{
		return true;
	}

	bool evar_system::shutdown()
	{
		return true;
	}

	bool evar_system::create_int(const std::string& name, int32_t value)
	{
		if (state->registered_ints_.contains(name))
		{
			LOG_WARN("Evar {} is already registered as an int", name);
			return false;
		}

		state->registered_ints_.emplace(name, value);
		return true;
	}

	int32_t evar_system::get_int(const std::string& name)
	{
		if (state->registered_ints_.contains(name))
		{
			return state->registered_ints_[name];
		}
		LOG_ERROR("Evar {} is not registered", name);
		return 0;
	}

	void evar_system::set_int(const std::string& name, int32_t value)
	{
		if (state->registered_ints_.contains(name))
		{
			state->registered_ints_[name] = value;
			event_context context{};
			context.set(0, value);
			event::fire_event(event_code::evar_changed, nullptr, context);
			return;
		}
		LOG_ERROR("Evar {} is not registered", name);

	}

	void evar_system::create_int_command(const console::context& context)
	{
		if (context.arguments.size() != 2)
		{
			LOG_ERROR("Invalid number of arguments for create int command");
			return;
		}

		create_int(context.arguments[0].value, std::stoi(context.arguments[1].value));
	}

	void evar_system::print_int_command(const console::context& context)
	{
		if (context.arguments.size() != 1)
		{
			LOG_ERROR("Invalid number of arguments for print int command. Got {}, expected 1", context.arguments.size());
			return;
		}

		auto value = get_int(context.arguments[0].value);
		console::write_line(nullptr, log_level::info, std::to_string(value));
	}
}