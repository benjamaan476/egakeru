#pragma once
#include "pch.h"

namespace egkr
{
	class platform
	{
	public:
		struct dynamic_library
		{
			struct function
			{
				std::string name;
				void* pfn;
			};

			uint64_t size{0};
			void* internal_data{nullptr};
			std::string library_name;
			std::string filename;
			egkr::vector<function> functions;
		};

		struct configuration
		{
			uint32_t start_x{};
			uint32_t start_y{};
			uint32_t width_{};
			uint32_t height_{};
			std::string name;
		};
		using shared_ptr = std::shared_ptr<platform>;
		static shared_ptr create(const platform::configuration& configuration);

		virtual ~platform() = default;

		virtual void shutdown() = 0;

		virtual void pump() = 0;
		[[nodiscard]] virtual bool is_running() const = 0;

		[[nodiscard]] virtual std::chrono::nanoseconds get_time() const = 0;
		virtual void sleep(std::chrono::nanoseconds duration) const = 0;

		[[nodiscard]] virtual egkr::vector<const char*> get_required_extensions() const = 0;

		[[nodiscard]] virtual void* get_window() const = 0;

		virtual uint2 get_framebuffer_size() = 0;

		[[nodiscard]] static std::optional<dynamic_library> load_library(const std::string& library_name);
		static bool unload_library(dynamic_library& library);
		static bool load_function(const std::string& function_name, dynamic_library& library);
	};
}
