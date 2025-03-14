#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

#include "resources/texture.h"
#include "vulkan_renderbuffer.h"
#include "command_buffer.h"

namespace egkr
{
    struct vulkan_context;
    class vulkan_texture : public texture
    {
    public:
	struct properties
	{
	    vk::Format image_format{};
	    vk::ImageTiling tiling{};
	    vk::ImageUsageFlags usage;
	    vk::MemoryPropertyFlags memory_properties;
	    vk::ImageAspectFlags aspect_flags;

	    texture::type texture_type{};

	    //TODO: support configurable depth, mip levels, layers, sample count, sharing mode
	    vk::SampleCountFlagBits samples{vk::SampleCountFlagBits::e1};
	    vk::SharingMode sharing_mode{vk::SharingMode::eExclusive};

	    uint32_t depth{1};
	    uint32_t mip_levels{1};
	};
	using shared_ptr = std::shared_ptr<vulkan_texture>;
	static vulkan_texture::shared_ptr create(const vulkan_context* context, uint32_t width, uint32_t height, const egkr::texture::properties& properties);
	static vulkan_texture create_raw(const vulkan_context* context, uint32_t width, uint32_t height, const egkr::texture::properties& properties);
	void create_view(const properties& properties);

	vulkan_texture();
	vulkan_texture(const vulkan_context* context, uint32_t width_, uint32_t height_, const egkr::texture::properties& properties);
	~vulkan_texture() override;

	bool populate(const egkr::texture::properties& properties, const uint8_t* data) override;
	bool populate_writeable() override;
	void populate_internal(const properties& properties);
	bool generate_mipmaps(const command_buffer& command_buffer, vk::Format image_format, uint32_t mip_levels);

	bool write_data(uint64_t offset, uint64_t size, const uint8_t* data) override;
	void read_data(uint64_t offset, uint64_t size, void* out_memory) override;
	void read_pixel(uint32_t x, uint32_t y, uint4* out_rgba) override;
	bool resize(uint32_t width, uint32_t height) override;
	void free() override;

	void destroy();

	[[nodiscard]] const auto& get_view() const { return view_; }

	void transition_layout(command_buffer command_buffer, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
	void copy_from_buffer(command_buffer command_buffer, vk::Buffer buffer);
	void copy_to_buffer(command_buffer command_buffer, vk::Buffer buffer);
	void copy_pixel_to_buffer(command_buffer command_buffer, vk::Buffer buffer, uint32_t x, uint32_t y);

	[[nodiscard]] const auto& get_image() const { return image_; }
	void set_image(vk::Image image);

	void set_view(vk::ImageView view);
	void set_width(uint32_t width) { width_ = width; }
	void set_height(uint32_t height) { height_ = height; }
    private:
	const vulkan_context* context_{};
	vk::Image image_;
	vk::DeviceMemory memory_;
	// vk::Format format_;
	vk::ImageView view_;
	uint32_t width_{};
	uint32_t height_{};
	uint32_t mip{1};
    };

    class vulkan_texture_map : public texture_map
    {
    public:
	using shared_ptr = std::shared_ptr<texture_map>;
	static shared_ptr create(const vulkan_context* context, const egkr::texture_map::properties& properties) { return std::make_shared<vulkan_texture_map>(context, properties); }

	vulkan_texture_map(const vulkan_context* context, const egkr::texture_map::properties& properties);
	~vulkan_texture_map() override;
	vk::Sampler create_sampler();
	void acquire() override;
	void release() override;
	bool refresh() override;

	vk::Sampler sampler;
    private:
	const vulkan_context* context_{};
    };
}
