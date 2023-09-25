#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	class fence
	{
	public:
		using shared_ptr = std::shared_ptr<fence>;
		static shared_ptr create(const vulkan_context* context, bool is_signaled);

		fence(const vulkan_context* context, bool is_signaled);
		~fence();

		bool wait(std::chrono::nanoseconds timeout);
		void reset();

		void destroy();
		const auto& get_handle() const { return fence_; }
	private:
		const vulkan_context* context_{};
		vk::Fence fence_{};
		bool is_signaled_{};
	};
}