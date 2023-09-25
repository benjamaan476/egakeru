#pragma once
#include "pch.h"

#include "platform/platform.h"

namespace egkr
{
	enum class backend_type
	{
		vulkan,
		opengl,
		directx
	};

	struct render_packet
	{
		std::chrono::milliseconds delta_time{};
	};

	class renderer_backend
	{
	public:
		using unique_ptr = std::unique_ptr<renderer_backend>;

		virtual ~renderer_backend() = default;
		virtual bool init() = 0;
		virtual void shutdown() = 0;
		virtual void resize(uint32_t width_, uint32_t height_) = 0;
		virtual void begin_frame(std::chrono::milliseconds delta_time) = 0;
		virtual void end_frame() = 0;
	private:
		platform::shared_ptr platform_;
	};


}