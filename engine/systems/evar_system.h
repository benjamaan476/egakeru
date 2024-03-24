#pragma once
#include "pch.h"

#include "system.h"
#include "console_system.h"

namespace egkr
{
	class evar_system : public system
	{
	public:
		using unique_ptr = std::unique_ptr<evar_system>;
		static evar_system* create();

		evar_system() = default;

		bool init() override;
		bool update(float delta_time) override;
		bool shutdown() override;

		static bool create_int(const std::string& name, int32_t value);
		static int32_t get_int(const std::string& name);
		static void set_int(const std::string& name, int32_t value);

		static void create_int_command(const console::context& context);
		static void print_int_command(const console::context& context);

	private:

		std::unordered_map<std::string, int32_t> registered_ints_{};

	};
}