#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

#include "buffer.h"
#include "command_buffer.h"

namespace egkr
{
	struct vulkan_context;
	namespace image
	{
		struct properties
		{
			vk::Format image_format{};
			vk::ImageTiling tiling{};
			vk::ImageUsageFlags usage{};
			vk::MemoryPropertyFlags memory_properties{};
			vk::ImageAspectFlags aspect_flags{};

			//TODO: support configurable depth, mip levels, layers, sample count, sharing mode
			const vk::SampleCountFlagBits samples{ vk::SampleCountFlagBits::e1 };
			const vk::SharingMode sharing_mode{ vk::SharingMode::eExclusive };

			const uint32_t depth{ 1 };
			const uint32_t mip_levels{ 4 };
			const uint32_t array_layers{ 1 };
		};

		class image;
		using shared_ptr = std::shared_ptr<image>;
		class image
		{
		public:
			API static shared_ptr create(const vulkan_context* context, uint32_t width_, uint32_t height_, const properties& properties, bool create_view);
			static image* create_raw(const vulkan_context* context, uint32_t width, uint32_t height, const properties& properties, bool create_view);
			void create_view(const properties& properties);

			image(const vulkan_context* context, uint32_t width_, uint32_t height_, const properties& properties);
			~image();

			void destroy();

			const auto& get_view() const
			{
				return view_;
			}

			void transition_layout(command_buffer command_buffer, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
			void copy_from_buffer(command_buffer command_buffer, buffer::shared_ptr buffer);

			[[nodiscard]] const auto& get_image() const
			{
				return image_;
			}
			void set_image(vk::Image image)
			{
				image_ = image;
			}

			void set_view(vk::ImageView view)
			{
				view_ = view;
			}
			void set_width(uint32_t width)
			{
				width_ = width;
			}
			void set_height(uint32_t height)
			{
				height_ = height;
			}

		private:
			const vulkan_context* context_{};
			vk::Image image_{};
			vk::DeviceMemory memory_{};
			vk::ImageView view_{};
			uint32_t width_{};
			uint32_t height_{};
		};
	}
}