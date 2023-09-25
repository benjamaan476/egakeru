#pragma once

#include "pch.h"

#include "renderpass.h"

namespace egkr
{
	struct framebuffer_properties
	{
		std::array<vk::ImageView, 2> attachments{};
		renderpass::shared_ptr renderpass{};
		uint32_t width_{};
		uint32_t height_{};
	};

	struct vulkan_context;
	class framebuffer
	{
	public:
		using unique_ptr = std::unique_ptr<framebuffer>;
		static unique_ptr create(const vulkan_context* context, const framebuffer_properties& properties);

		framebuffer(const vulkan_context* context, const framebuffer_properties& properties);
		~framebuffer();

		void destroy();
	private:
		const vulkan_context* context_{};
		vk::Framebuffer framebuffer_{};

		std::array<vk::ImageView, 2> attachments_{};
		renderpass::shared_ptr renderpass_{};
		uint32_t width_{};
		uint32_t height_{};

	};
}