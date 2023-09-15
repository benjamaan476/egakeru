#pragma once
#include "pch.h"

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

	struct platform_configuration
	{
		uint32_t start_x{};
		uint32_t start_y{};
		uint32_t width{};
		uint32_t height{};
		std::string name{};
	};


	class platform : public std::enable_shared_from_this<platform>
	{
	public:
		using shared_ptr = std::shared_ptr<platform>;
		static shared_ptr create(platform_type type);

		virtual ~platform() = default;

		virtual bool startup(const platform_configuration& configuration) = 0;
		virtual void shutdown() = 0;

		virtual void pump() = 0;
		virtual bool is_running() const = 0;

		virtual std::chrono::milliseconds get_time() const = 0;
		virtual void sleep(std::chrono::milliseconds duration) const = 0;
	};
}