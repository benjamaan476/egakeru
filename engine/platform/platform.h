#pragma once
#include "pch.h"
#include <vulkan/vulkan.hpp>
#include <expected>

namespace egkr
{
	class platform : public std::enable_shared_from_this<platform>
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
		static shared_ptr create();

		virtual ~platform() = default;

		virtual bool startup(const configuration& configuration) = 0;
		virtual void shutdown() = 0;

		virtual void pump() = 0;
		virtual bool is_running() const = 0;

		virtual std::chrono::nanoseconds get_time() const = 0;
		virtual void sleep(std::chrono::nanoseconds duration) const = 0;

		virtual egkr::vector<const char*> get_required_extensions() const = 0;
		virtual vk::SurfaceKHR create_surface(vk::Instance instance) = 0;

		virtual uint2 get_framebuffer_size() = 0;
		// virtual std::expected<dynamic_library, void> load_library(const std::string& library_name) const = 0;
		// virtual bool unload_library(dynamic_library& library) const = 0;
		// virtual bool load_function(const std::string& function_name, dynamic_library& library) const = 0;
	};
}
