#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

#include "resources/texture.h"
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

			texture::type texture_type{};

			//TODO: support configurable depth, mip levels, layers, sample count, sharing mode
			const vk::SampleCountFlagBits samples{ vk::SampleCountFlagBits::e1 };
			const vk::SharingMode sharing_mode{ vk::SharingMode::eExclusive };

			const uint32_t depth{ 1 };
			const uint32_t mip_levels{ 4 };
		};

		class vulkan_texture : public texture::texture
		{
		public:
			static vulkan_texture* create(const vulkan_context* context, uint32_t width_, uint32_t height_, const egkr::texture::properties& properties, bool create_view);
			static vulkan_texture* create_raw(const vulkan_context* context, uint32_t width, uint32_t height, const egkr::texture::properties& properties, bool create_view);
			void create_view(const properties& properties);

			vulkan_texture();
			vulkan_texture(const vulkan_context* context, uint32_t width_, uint32_t height_, const egkr::texture::properties& properties);
			~vulkan_texture() override;

			bool populate(const egkr::texture::properties& properties, const uint8_t* data) override;
			bool populate_writeable() override;
			bool write_data(uint64_t offset, uint32_t size, const uint8_t* data) override;
			bool resize(uint32_t width, uint32_t height) override;
			void free() override;

			void destroy();

			const auto& get_view() const { return view_; }

			void transition_layout(command_buffer command_buffer, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
			void copy_from_buffer(command_buffer command_buffer, vulkan_buffer::shared_ptr buffer);

			[[nodiscard]] const auto& get_image() const { return image_; }
			void set_image(vk::Image image) { image_ = image; }

			void set_view(vk::ImageView view);
			void set_width(uint32_t width) { width_ = width; }
			void set_height(uint32_t height) { height_ = height; }
			void create(const image::properties& properties);

		private:
			const vulkan_context* context_{};
			vk::Image image_{};
			vk::DeviceMemory memory_{};
			vk::ImageView view_{};
			uint32_t width_{};
			uint32_t height_{};
		};
	}

	namespace texture::vulkan::texture_map
	{
		class texture_map : public egkr::texture_map::texture_map
		{
		public:
			using shared_ptr = std::shared_ptr<texture_map>;
			static shared_ptr create(const vulkan_context* context, const egkr::texture_map::properties& properties)
			{
				return std::make_shared<texture_map>(context, properties);
			}

			texture_map(const vulkan_context* context, const egkr::texture_map::properties& properties);
			~texture_map() override;
			void acquire() override;
			void release() override;

			vk::Sampler sampler{};
		private:
			const vulkan_context* context_{};
		};
	}
}