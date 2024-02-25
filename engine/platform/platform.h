#pragma once
#include "pch.h"
#include <vulkan/vulkan.hpp>

namespace egkr
{

	enum class platform_type
	{
		windows,
		linux,
		macos,
		android
	};

	inline std::string_view platform_type_to_string(platform_type type)
	{
		switch (type)
		{
		case egkr::platform_type::windows:
			return "windows";
		case egkr::platform_type::linux:
			return "linux";
		case egkr::platform_type::macos:
			return "macos";
		case egkr::platform_type::android:
			return "android";
		default:
			return "invalid platform";
		}
	}

	class platform : public std::enable_shared_from_this<platform>
	{
	public:
		struct configuration
		{
			uint32_t start_x{};
			uint32_t start_y{};
			uint32_t width_{};
			uint32_t height_{};
			std::string name{};
		};
		using shared_ptr = std::shared_ptr<platform>;
		static shared_ptr create(platform_type type);

		virtual ~platform() = default;

		virtual bool startup(const configuration& configuration) = 0;
		virtual void shutdown() = 0;

		virtual void pump() = 0;
		virtual bool is_running() const = 0;

		virtual std::chrono::nanoseconds get_time() const = 0;
		virtual void sleep(std::chrono::nanoseconds duration) const = 0;

		virtual egkr::vector<const char*> get_required_extensions() const = 0;
		virtual vk::SurfaceKHR create_surface(vk::Instance instance) = 0;

		virtual int2 get_framebuffer_size() = 0;
	};
}