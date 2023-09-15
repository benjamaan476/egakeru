#pragma once
#include "pch.h"

#include "platform/platform.h"

namespace egkr
{
	struct app_state
	{
		bool is_running{};
		bool is_suspended{};
		platform::shared_ptr platform{};
		uint32_t width{};
		uint32_t height{};
	};

	struct application_configuration
	{
		uint32_t width{ 800 };
		uint32_t height{ 600 };
		std::string name{};
	};

	class application
	{
	public:
		using shared_ptr = std::shared_ptr<application>;
		API static shared_ptr create(const application_configuration& config);
		//Don't call this directly, only here to shutup clang
		explicit application(const application_configuration& config);

		API void run() const;

	private:

		inline static bool is_initialised_{false};
		app_state state_{};
	};
}