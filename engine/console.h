#pragma once
#include "pch.h"

namespace egkr
{
	using consumer_callback = std::function<bool(void*, log_level, std::string)>;

	class console
	{
	public:
		struct argument
		{
			std::string value{};
		};

		struct context
		{
			egkr::vector<argument> arguments{};
		};

		using command_callback= std::function<void(const context&)>;

		struct consumer
		{
			void* instance{};
			consumer_callback callback{};
		};

		struct command
		{
			std::string name{};
			uint8_t arg_count{};
			command_callback callback{};
		};

		using unique_ptr = std::unique_ptr<console>;
		static console* create();

		static void init();
		static void shutdown();

		static void register_consumer(void* instance, consumer_callback callback);
		static void write_line(void* instance, log_level level, const std::string& message);

		static bool register_command(const std::string& command, uint8_t argument_count, command_callback callback);
		static bool execute_command(const std::string& command);

	private:
		egkr::vector<consumer> consumers_{};
		egkr::vector<command> registered_commands_{};
	};

}