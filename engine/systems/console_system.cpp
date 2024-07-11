#include "console_system.h"
#include "evar_system.h"

namespace egkr
{
	static console::unique_ptr state{};

	console* console::create()
	{
		state = std::make_unique<console>();

		return state.get();
	}

	bool console::init()
	{
		register_command("evar_create_int", 2, evar_system::create_int_command);
		register_command("evar_print_int", 1, evar_system::print_int_command);
		return true;
	}

	bool console::shutdown()
	{
		if (state)
		{
			state->consumers_.clear();
			state->registered_commands_.clear();
			state.reset();
		}
		return true;
	}

	void console::register_consumer(void* instance, consumer_callback callback)
	{
		state->consumers_.emplace_back(instance, callback);
	}

	void console::write_line(void* instance, log_level level, const std::string& message)
	{
		for (const auto& consumer : state->consumers_)
		{
			consumer.callback(instance, level, message);
		}
	}

	bool console::register_command(const std::string& command, uint8_t argument_count, command_callback callback)
	{
		for (auto& cmd : state->registered_commands_)
		{
			if (cmd.name == command)
			{
				LOG_WARN("Already registered command with this name");
				return false;
			}
		}

		state->registered_commands_.emplace_back(command, argument_count, callback);
		return true;
	}

	bool console::execute_command(const std::string& command)
	{
		std::istringstream ss{ command };
		std::string name{};
		ss >> name;

		egkr::vector<std::string> args{};
		std::string argument{};

		while(ss >> argument)
		{
			args.push_back(argument);
		}

		write_line(nullptr, log_level::info, command);

		bool has_error{};
		bool command_found{};

		for (const auto& cmd : state->registered_commands_)
		{
			if (cmd.name == name)
			{
				command_found = true;
				if (cmd.arg_count != args.size())
				{
					LOG_ERROR("Command found but argument count differs from registered command");
					has_error = true;
				}
				else
				{
					context context{};
					for (const auto& arg : args)
					{
						context.arguments.push_back({ arg });
					}
					cmd.callback(context);
				}
				break;
			}
		}

		if (!command_found)
		{
			LOG_ERROR("Command does not exist");
		}
		return !has_error;
	}
}
